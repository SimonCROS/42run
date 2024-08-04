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

#define RESOURCE_PATH "./resources/"

#include "stb_image.h"

const GLuint WIDTH = 1440 / 2, HEIGHT = 846 - 80;
GLuint whiteTexture = 0;

struct RendererState
{
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 lightPos;
};

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char *message,
                            const void *userParam)
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

static void CheckErrors(const std::string_view &desc)
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

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static inline void *BufferOffset(const size_t offset)
{
    return reinterpret_cast<void *>(offset);
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

static bool BindTexture(const std::map<int, GLuint> &textures, const int textureIndex, const ShaderProgram &program, const std::string_view &bindingKey, const int bindingValue)
{
    if (textureIndex < 0)
    {
        return false;
    }

    if (textures.count(textureIndex) != 0)
    {
        GLuint glTexture = textures.at(textureIndex);

        glActiveTexture(GL_TEXTURE0 + bindingValue);
        program.SetInt(bindingKey.data(), bindingValue);
        glBindTexture(GL_TEXTURE_2D, glTexture > 0 ? glTexture : whiteTexture);
    } // TODO Else throw ?

    return true;
}

static void DrawMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const std::map<int, GLuint> &buffers,
                     const std::map<int, GLuint> &textures, ShaderProgramVariants &programVariants, RendererState &state, glm::dmat4 transform)
{
    for (const auto &primitive : mesh.primitives)
    {
        if (primitive.indices < 0)
            continue;

        ShaderProgram &program = programVariants.GetProgram(GetPrimitiveShaderFlags(model, primitive));
        program.Use();

        for (const auto &[attribute, accessorId] : primitive.attributes)
        {
            assert(accessorId >= 0);

            const tinygltf::Accessor &accessor = model.accessors[accessorId];

            glBindBuffer(GL_ARRAY_BUFFER, buffers.at(accessor.bufferView));
            CheckErrors("bind buffer");

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
            const auto &material = model.materials[primitive.material];
            BindTexture(textures, material.pbrMetallicRoughness.baseColorTexture.index, program, "albedoMap", 0);
            BindTexture(textures, material.pbrMetallicRoughness.metallicRoughnessTexture.index, program, "metallicRoughnessMap", 1);
            BindTexture(textures, material.normalTexture.index, program, "normalMap", 2);
            BindTexture(textures, material.emissiveTexture.index, program, "emissiveMap", 3);
            program.SetFloat("metallicFactor", static_cast<float>(material.pbrMetallicRoughness.metallicFactor));
            program.SetFloat("roughnessFactor", static_cast<float>(material.pbrMetallicRoughness.roughnessFactor));
            program.SetVec3("emissiveFactor", glm::make_vec3(material.emissiveFactor.data()));
            program.SetVec4("color", glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data()));
            if (material.normalTexture.index >= 0)
            {
                program.SetFloat("normalScale", static_cast<float>(material.normalTexture.scale));
            }
        }

        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.at(indexAccessor.bufferView));
        CheckErrors("bind buffer");

        int mode = GetDrawMode(primitive.mode);
        assert(mode != -1);

        glDrawElements(mode, static_cast<GLsizei>(indexAccessor.count), indexAccessor.componentType,
                       BufferOffset(indexAccessor.byteOffset));
        CheckErrors("draw elements");
    }
}

static void DrawNode(tinygltf::Model &model, const tinygltf::Node &node, const std::map<int, GLuint> &buffers,
                     const std::map<int, GLuint> &textures, ShaderProgramVariants &programVariants, RendererState &state, glm::dmat4 transform)
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
    for (const int &child : node.children)
    {
        DrawNode(model, model.nodes[child], buffers, textures, programVariants, state, transform);
    }
}

float wrapAngle(float angle) {
    angle = fmod(angle + 180.0f, 360.0f);
    if (angle < 0) {
        angle += 360.0f;
    }
    return angle - 180.0f;
}

glm::vec3 getHorizontalDirection(const glm::vec3& v) {
    glm::vec3 horizontalDir(v.x, 0.0f, v.z);
    return glm::normalize(horizontalDir);
}

void processInput(GLFWwindow *window, RendererState& state)
{
    const float cameraSpeed = 2.4f * state.deltaTime;
    const float cameraRotationSpeed = 60.0f * state.deltaTime;
    const glm::vec3 horizontalDirection = getHorizontalDirection(state.cameraFront);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        state.cameraPos += cameraSpeed * horizontalDirection;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        state.cameraPos -= cameraSpeed * horizontalDirection;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        state.cameraPos -= glm::normalize(glm::cross(horizontalDirection, state.cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        state.cameraPos += glm::normalize(glm::cross(horizontalDirection, state.cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        state.cameraPos -= glm::vec3(0, cameraSpeed, 0);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        state.cameraPos += glm::vec3(0, cameraSpeed, 0);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        state.pitch += cameraRotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        state.pitch -= cameraRotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        state.yaw += cameraRotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        state.yaw -= cameraRotationSpeed;
    state.pitch = std::clamp(state.pitch, -89.0f, 89.0f);
    state.yaw = wrapAngle(state.yaw);

    const float radPitch = glm::radians(state.pitch);
    const float radYaw = glm::radians(state.yaw);
    const float cPitch = cos(radPitch);
    const float sPitch = sin(radPitch);
    const float cYaw = -cos(radYaw);
    const float sYaw = -sin(radYaw);

    glm::vec3 direction;
    direction.x = sYaw * cPitch;
    direction.y = sPitch;
    direction.z = cYaw * cPitch;
    state.cameraFront = glm::normalize(direction);
}

static int run(GLFWwindow *window)
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

    // ModelLoader loader(RESOURCE_PATH "sea_house.glb");
    // ModelLoader loader(RESOURCE_PATH "brick_wall_test/scene.gltf");
    // ModelLoader loader(RESOURCE_PATH "goshingyu/scene.gltf");
    // ModelLoader loader(RESOURCE_PATH "metal_dragon.glb");
    // ModelLoader loader(RESOURCE_PATH "magic_laboratory.glb");
    // ModelLoader loader(RESOURCE_PATH "Cube/Cube.gltf");
    // ModelLoader loader(RESOURCE_PATH "buster_drone/scene.gltf");
    // ModelLoader loader(RESOURCE_PATH "buster_drone.glb");
    ModelLoader loader(RESOURCE_PATH "minecraft_castle.glb");
    // ModelLoader loader(RESOURCE_PATH "free_porsche_911_carrera_4s.glb");
    // ModelLoader loader(RESOURCE_PATH "girl_speedsculpt.glb");

    loader.LoadAsync();

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

    auto transform = glm::identity<glm::dmat4>();
    transform = glm::scale(transform, glm::dvec3(0.2));
    transform = glm::rotate(transform, glm::radians(45.0), glm::dvec3(0.0, 1.0, 0.0));
    transform = glm::translate(transform, glm::dvec3(0.0, -6, 2));

    while (!glfwWindowShouldClose(window) && !loader.IsCompleted())
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(window);
    }

    if (loader.IsError())
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    loader.Prepare();

    if (!programVariants.EnableVariants(loader.usedShaderFlagCombinations))
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    glm::vec3 lightPos = glm::vec3(5, 50, 40);
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 1000.0f);

    RendererState state {
        .cameraPos = glm::vec3(0.0f, 3, 5),
        .cameraFront = glm::vec3(0.0f, 0.0f, -1.0f),
        .cameraUp = glm::vec3(0.0f, 1.0f,  0.0f),
        .projection = proj,
        .lightPos = lightPos,
    };

    while (!glfwWindowShouldClose(window))
    {
        auto currentFrame = static_cast<float>(glfwGetTime());
        state.deltaTime = currentFrame - state.lastFrame;
        state.lastFrame = currentFrame;

        processInput(window, state);
        state.view = glm::lookAt(state.cameraPos, state.cameraPos + state.cameraFront, state.cameraUp);

        glfwPollEvents();

        if (!loader.IsCompleted())
        {
            glfwSwapBuffers(window);
            continue;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(loader.vao);

        for (auto &[flags, program] : programVariants.programs)
        {
            program.Use();
            program.SetVec3("viewPos", state.cameraPos);
            program.SetMat4("projection", proj);
            program.SetMat4("view", state.view);
            program.SetVec3("lightPos", lightPos);
        }

        const auto &scene = loader.model.scenes[loader.model.defaultScene];
        for (const int &node : scene.nodes)
        {
            DrawNode(loader.model, loader.model.nodes[node], loader.buffers, loader.textures, programVariants, state, transform);
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        // transform = glm::rotate(transform, glm::radians(0.3), glm::dvec3(0.0, 1.0, 0.0));
    }

    loader.Wait();

    programVariants.Destroy();
    glDeleteTextures(1, &whiteTexture);
    for (auto &[id, texture] : loader.textures)
    {
        glDeleteTextures(1, &texture);
    }
    for (auto &[id, buffer] : loader.buffers)
    {
        glDeleteBuffers(1, &buffer);
    }
    glDeleteVertexArrays(1, &loader.vao);

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
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "42run", nullptr, nullptr);
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
