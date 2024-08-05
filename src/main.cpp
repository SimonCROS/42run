#include <iostream>

#include "glad/gl.h"

#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>

#include "Shader.hpp"
#include "ShaderProgram.hpp"
#include "ShaderProgramVariants.hpp"
#include "Model.hpp"
#include "ModelLoader.hpp"
#include "Camera.hpp"
#include "stb_image.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"

#define RESOURCE_PATH "./resources/"

const GLuint WIDTH = 1440 / 2, HEIGHT = 846 - 80;
GLuint whiteTexture = 0;

#define CUSTOM_MAX_BINDED_TEXTURES 16

struct RendererState
{
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    Camera camera;
    glm::mat4 projection;
    glm::vec3 lightPos;
    GLuint bindedVertexBuffer;
    GLuint bindedElementBuffer;
    GLuint bindedTextures[CUSTOM_MAX_BINDED_TEXTURES];
};

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char* message,
                            const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "Source: Other";
        break;
    }
    std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other";
        break;
    }
    std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

static void CheckErrors(const std::string_view& desc)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error in \"" << desc << "\": " << e << std::endl;
        exit(20);
    }
}

static GLuint CreateWhiteTexture()
{
    int data[4] = {~0, ~0, ~0, ~0};
    GLuint whiteTextureId;
    glGenTextures(1, &whiteTextureId);
    glBindTexture(GL_TEXTURE_2D, whiteTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    return whiteTextureId;
}

static void key_callback(GLFWwindow* window, const int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void* BufferOffset(const size_t offset)
{
    return reinterpret_cast<void*>(offset);
}

static int GetDrawMode(int tinygltfMode)
{
    if (tinygltfMode == TINYGLTF_MODE_TRIANGLES)
        return GL_TRIANGLES;
    else if (tinygltfMode == TINYGLTF_MODE_TRIANGLE_STRIP)
        return GL_TRIANGLE_STRIP;
    else if (tinygltfMode == TINYGLTF_MODE_TRIANGLE_FAN)
        return GL_TRIANGLE_FAN;
    else if (tinygltfMode == TINYGLTF_MODE_POINTS)
        return GL_POINTS;
    else if (tinygltfMode == TINYGLTF_MODE_LINE)
        return GL_LINES;
    else if (tinygltfMode == TINYGLTF_MODE_LINE_LOOP)
        return GL_LINE_LOOP;
    else
        return -1;
}

static void BindVertexBuffer(RendererState& state, GLuint buffer)
{
    if (state.bindedVertexBuffer != buffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        CheckErrors("bind buffer");
        state.bindedVertexBuffer = buffer;
    }
}

static void BindElementBuffer(RendererState& state, GLuint buffer)
{
    if (state.bindedElementBuffer != buffer)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        CheckErrors("bind buffer");
        state.bindedElementBuffer = buffer;
    }
}

static bool BindTexture(RendererState& state, const std::unordered_map<int, GLuint>& textures, const int textureIndex,
                        ShaderProgram& program, const std::string_view& bindingKey, const GLuint bindingValue)
{
    assert(bindingValue < CUSTOM_MAX_BINDED_TEXTURES);

    if (textureIndex < 0)
    {
        return false; // TODO throw ?
    }

    if (textures.count(textureIndex) != 0)
    {
        GLuint glTexture = textures.at(textureIndex);

        // SetInt on program before, if the shader has changed
        program.SetInt(bindingKey.data(), bindingValue);

        if (state.bindedTextures[bindingValue] == glTexture)
        {
            return true;
        }

        glActiveTexture(GL_TEXTURE0 + bindingValue);
        glBindTexture(GL_TEXTURE_2D, glTexture > 0 ? glTexture : whiteTexture);
        state.bindedTextures[bindingValue] = glTexture;
    } // TODO Else throw ?

    return true;
}

static void DrawMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh,
                     const std::unordered_map<int, GLuint>& buffers,
                     const std::unordered_map<int, GLuint>& textures, ShaderProgramVariants& programVariants,
                     RendererState& state, const glm::dmat4& transform)
{
    for (const auto& primitive : mesh.primitives)
    {
        if (primitive.indices < 0)
            continue;

        ShaderProgram& program = programVariants.GetProgram(GetPrimitiveShaderFlags(model, primitive));
        program.Use();

        for (const auto& [attribute, accessorId] : primitive.attributes)
        {
            assert(accessorId >= 0);

            const tinygltf::Accessor& accessor = model.accessors[accessorId];

            BindVertexBuffer(state, buffers.at(accessor.bufferView));

            int size = tinygltf::GetNumComponentsInType(accessor.type);
            assert(size != -1);

            int attributeLocation = program.GetAttributeLocation(attribute);
            if (attributeLocation != -1)
            {
                int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
                assert(byteStride != -1);

                glVertexAttribPointer(attributeLocation, size, accessor.componentType, accessor.normalized,
                                      byteStride, BufferOffset(accessor.byteOffset));
                CheckErrors("vertex attrib pointer");

                program.EnableAttribute(attributeLocation);
                CheckErrors("enable vertex attrib array");
            }
        }

        program.ApplyAttributeChanges();
        program.SetMat4("transform", transform);

        if (primitive.material >= 0)
        {
            const auto& material = model.materials[primitive.material];
            BindTexture(state, textures, material.pbrMetallicRoughness.baseColorTexture.index, program, "albedoMap", 0);
            BindTexture(state, textures, material.pbrMetallicRoughness.metallicRoughnessTexture.index, program,
                        "metallicRoughnessMap", 1);
            BindTexture(state, textures, material.normalTexture.index, program, "normalMap", 2);
            BindTexture(state, textures, material.emissiveTexture.index, program, "emissiveMap", 3);
            program.SetFloat("metallicFactor", static_cast<float>(material.pbrMetallicRoughness.metallicFactor));
            program.SetFloat("roughnessFactor", static_cast<float>(material.pbrMetallicRoughness.roughnessFactor));
            program.SetVec3("emissiveFactor", glm::make_vec3(material.emissiveFactor.data()));
            program.SetVec4("color", glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data()));
            if (material.normalTexture.index >= 0)
            {
                program.SetFloat("normalScale", static_cast<float>(material.normalTexture.scale));
            }
        }

        const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];

        BindElementBuffer(state, buffers.at(indexAccessor.bufferView));

        const int mode = GetDrawMode(primitive.mode);
        assert(mode != -1);

        glDrawElements(mode, static_cast<GLsizei>(indexAccessor.count), indexAccessor.componentType,
                       BufferOffset(indexAccessor.byteOffset));
        CheckErrors("draw elements");
    }
}

static void DrawNode(tinygltf::Model& model, const tinygltf::Node& node, const std::unordered_map<int, GLuint>& buffers,
                     const std::unordered_map<int, GLuint>& textures, ShaderProgramVariants& programVariants,
                     RendererState& state, glm::dmat4 transform)
{
    if (node.matrix.size() == 16)
    {
        transform *= glm::make_mat4(node.matrix.data());
    }
    else
    {
        if (node.translation.size() == 3)
        {
            transform = glm::translate(transform, glm::make_vec3(node.translation.data()));
        }

        if (node.rotation.size() == 4)
        {
            transform *= glm::mat4_cast(glm::make_quat(node.rotation.data()));
        }

        if (node.scale.size() == 3)
        {
            transform = glm::scale(transform, glm::make_vec3(node.scale.data()));
        }
    }

    if (node.mesh >= 0 && node.mesh < model.meshes.size())
    {
        DrawMesh(model, model.meshes[node.mesh], buffers, textures, programVariants, state, transform);
    }
    for (const int& child : node.children)
    {
        DrawNode(model, model.nodes[child], buffers, textures, programVariants, state, transform);
    }
}

void processInput(GLFWwindow* window, RendererState& state)
{
    const float cameraRotationSpeed = glm::radians(180.0f * state.deltaTime);
    Camera& camera = state.camera;
    const float speed = camera.Speed * state.deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Position += speed * camera.Direction;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.Position -= speed * camera.Direction;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.Position -= speed * camera.GetCrossDirection();
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.Position += speed * camera.GetCrossDirection();
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.Position -= glm::vec3(0, speed, 0);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.Position += glm::vec3(0, speed, 0);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.Direction = glm::rotate(camera.Direction, cameraRotationSpeed, camera.GetCrossDirection());
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.Direction = glm::rotate(camera.Direction, -cameraRotationSpeed, camera.GetCrossDirection());
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.Direction = glm::rotateY(camera.Direction, cameraRotationSpeed);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.Direction = glm::rotateY(camera.Direction, -cameraRotationSpeed);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        std::cout << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z << std::endl;
}

static int run(GLFWwindow* window)
{
    int version = gladLoadGL(glfwGetProcAddress);
    std::cout << "OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    //! Create shader program
    ShaderProgramVariants programVariants(RESOURCE_PATH "shaders/default.vert", RESOURCE_PATH "shaders/default.frag");

    whiteTexture = CreateWhiteTexture();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_MULTISAMPLE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glm::vec3 lightPos = glm::vec3(5, 50, 40);
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f,
                                      1000.0f);

    RendererState state{
        .projection = proj,
        .lightPos = lightPos,
        .camera = Camera(2.4f, glm::vec3(0, 0, 5.0f), glm::vec3(0, 0, -1)),
        // .camera = Camera(2.4f, glm::vec3(20.5, 0.5, 50), glm::vec3(0, 0, -1)),
        // .camera = Camera(2.4f, glm::vec3(20.5, 0.5, 29.5), glm::vec3(1, 0, 0)),
        .bindedVertexBuffer = 0,
        .bindedElementBuffer = 0,
        .bindedTextures = {0},
    };

    auto loaders = std::vector<ModelLoader>();
    loaders.reserve(42);
    auto models = std::vector<ModelLoader>(); // TODO Should not be ModelLoader
    models.reserve(42);

    // loaders.emplace_back(RESOURCE_PATH "sea_house.glb");
    // loaders.emplace_back(RESOURCE_PATH "brick_wall_test/scene.gltf");
    // loaders.emplace_back(RESOURCE_PATH "goshingyu/scene.gltf");
    // loaders.emplace_back(RESOURCE_PATH "metal_dragon.glb");
    // loaders.emplace_back(RESOURCE_PATH "magic_laboratory.glb");
    // loaders.emplace_back(RESOURCE_PATH "Cube/Cube.gltf");
    // loaders.emplace_back(RESOURCE_PATH "buster_drone/scene.gltf");
    // loaders.emplace_back(RESOURCE_PATH "buster_drone.glb");
    loaders.emplace_back(RESOURCE_PATH "minecraft_castle.glb");
    // loaders.emplace_back(RESOURCE_PATH "free_porsche_911_carrera_4s.glb");
    // loaders.emplace_back(RESOURCE_PATH "girl_speedsculpt.glb");
    // loaders.emplace_back(RESOURCE_PATH "low_poly_tree_scene_free.glb");

    for (auto& loader : loaders)
    {
        loader.LoadAsync();
    }

    glm::dmat4 transform;
    while (!glfwWindowShouldClose(window))
    {
        // BUILD LOADERS
        auto it = loaders.begin();
        while (it != loaders.end())
        {
            auto& loader = *it;
            if (loader.IsCompleted())
            {
                loader.Wait();

                if (!loader.IsError())
                {
                    loader.Prepare();
                    if (!loader.BuildShaders(programVariants))
                    {
                        loader.Destroy();
                    }
                    else
                    {
                        models.emplace_back(std::move(loader));
                    }
                }

                it = loaders.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // UPDATE VALUES
        auto currentFrame = static_cast<float>(glfwGetTime());
        state.deltaTime = currentFrame - state.lastFrame;
        state.lastFrame = currentFrame;

        processInput(window, state);

        glfwPollEvents();

        // DRAW
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (auto& model : models)
        {
            glBindVertexArray(model.vao);

            for (auto& [flags, program] : programVariants.programs)
            {
                program.Use();
                program.SetVec3("viewPos", state.camera.Position);
                program.SetMat4("projection", proj);
                program.SetMat4("view", state.camera.GetView());
                program.SetVec3("lightPos", lightPos);
            }

            const auto& scene = model.model.scenes[model.model.defaultScene];
            for (const int& node : scene.nodes)
            {
                DrawNode(model.model, model.model.nodes[node], model.buffers, model.textures, programVariants, state,
                         transform);
            }

            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
        // transform = glm::rotate(transform, glm::radians(0.3), glm::dvec3(0.0, 1.0, 0.0));
    }

    programVariants.Destroy();
    glDeleteTextures(1, &whiteTexture);
    for (auto& loader : loaders)
    {
        loader.Wait();
        loader.Destroy();
    }
    for (auto& model : models)
    {
        model.Destroy();
    }

    return 0;
}

int main()
{
    glfwInit();

    // GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    // const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    // std::cout << mode->width << " " << mode->height << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfwSetErrorCallback(error_callback);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "42run", nullptr, nullptr);
    if (window == nullptr)
    {
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    int ret = run(window);

    glfwTerminate();

    return ret;
}
