#include <iostream>

#include "glad/gl.h"

#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>

#include "Shader.hpp"
#include "ShaderProgram.hpp"
#include "Model.hpp"
#include "ModelLoader.hpp"

#define RESOURCE_PATH "./resources/"

#include "stb_image.h"

const GLuint WIDTH = 800, HEIGHT = 600;
GLuint whiteTexture = 0;

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

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static inline void* BufferOffset(const size_t offset)
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

static void DrawMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const std::map<int, GLuint>& buffers,
                     const std::map<int, GLuint>& textures, const ShaderProgram& program)
{
    for (const auto& primitive : mesh.primitives)
    {
        if (primitive.indices < 0)
            continue;

        for (const auto& [attribute, accessorId] : primitive.attributes)
        {
            assert(accessorId >= 0);

            const tinygltf::Accessor& accessor = model.accessors[accessorId];

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

                glEnableVertexAttribArray(attributeLocation);
                CheckErrors("enable vertex attrib array");
            }
        }

        if (primitive.material >= 0)
        {
            const auto& material = model.materials[primitive.material];
            int textureId = material.pbrMetallicRoughness.baseColorTexture.index;
            if (textureId >= 0 && textures.count(textureId) != 0)
            {
                glActiveTexture(GL_TEXTURE0 + material.pbrMetallicRoughness.baseColorTexture.texCoord);
                program.SetInt("texture0", material.pbrMetallicRoughness.baseColorTexture.texCoord);
                glBindTexture(GL_TEXTURE_2D, textures.at(textureId));
            }
            else
            {
                glActiveTexture(GL_TEXTURE0);
                program.SetInt("texture0", 0);
                glBindTexture(GL_TEXTURE_2D, whiteTexture);
            }
            program.SetVec4("texColor", glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data()));
        }

        const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.at(indexAccessor.bufferView));
        CheckErrors("bind buffer");

        int mode = GetDrawMode(primitive.mode);
        assert(mode != -1);

        glDrawElements(mode, static_cast<GLsizei>(indexAccessor.count), indexAccessor.componentType,
                       BufferOffset(indexAccessor.byteOffset));
        CheckErrors("draw elements");

        for (const auto& [attribute, accessorId] : primitive.attributes)
        {
            int attributeLocation = program.GetAttributeLocation(attribute);
            if (attributeLocation != -1)
            {
                glDisableVertexAttribArray(attributeLocation);
            }
        }
    }
}

static void DrawNode(tinygltf::Model& model, const tinygltf::Node& node, const std::map<int, GLuint>& buffers,
                     const std::map<int, GLuint>& textures, const ShaderProgram& program, glm::dmat4 transform)
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
        program.SetMat4("transform", transform);
        DrawMesh(model, model.meshes[node.mesh], buffers, textures, program);
    }
    for (const int& child : node.children)
    {
        DrawNode(model, model.nodes[child], buffers, textures, program, transform);
    }
}

static int run(GLFWwindow* window)
{
    int version = gladLoadGL(glfwGetProcAddress);
    std::cout << "OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;

    ModelLoader loader(RESOURCE_PATH "sea_house.glb");
    // ModelLoader loader(RESOURCE_PATH "magic_laboratory.glb");
    // ModelLoader loader(RESOURCE_PATH "PeterHeadSimpleHairMesh.glb");
    // ModelLoader loader(RESOURCE_PATH "Cube/Cube.gltf");
    // ModelLoader loader(RESOURCE_PATH "buster_drone/scene.gltf");
    // ModelLoader loader(RESOURCE_PATH "buster_drone.glb");
    // ModelLoader loader(RESOURCE_PATH "free_porsche_911_carrera_4s.glb");
    // ModelLoader loader(RESOURCE_PATH "girl_speedsculpt.glb");

    //! Create shader program
    const ShaderProgram program(
        Shader(RESOURCE_PATH "shaders/default.vert", GL_VERTEX_SHADER),
        Shader(RESOURCE_PATH "shaders/default.frag", GL_FRAGMENT_SHADER));

    whiteTexture = CreateWhiteTexture();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    auto transform = glm::identity<glm::dmat4>();
    //transform = glm::scale(transform, glm::dvec3(2.4));
    //transform = glm::rotate(transform, glm::radians(45.0), glm::dvec3(0.0, 1.0, 0.0));

    glm::vec3 cameraPos = glm::vec3(0.0f, 180, 250);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 50, 0.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!loader.IsCompleted())
        {
            glfwSwapBuffers(window);
            continue;
        }

        program.Use();
        program.SetVec3("camera", cameraPos);
        program.SetMat4("projection", proj);
        program.SetMat4("view", view);

        if (loader.IsError())
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
            continue;
        }

        glBindVertexArray(loader.vao);

        const auto& scene = loader.model.scenes[loader.model.defaultScene];
        for (const int& node : scene.nodes)
        {
            DrawNode(loader.model, loader.model.nodes[node], loader.buffers, loader.textures, program, transform);
        }

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        transform = glm::rotate(transform, glm::radians(0.2), glm::dvec3(0.0, 1.0, 0.0));
    }

    glDeleteTextures(1, &whiteTexture);
    for (auto& [id, texture] : loader.textures)
    {
        glDeleteTextures(1, &texture);
    }
    for (auto& [id, buffer] : loader.buffers)
    {
        glDeleteBuffers(1, &buffer);
    }
    glDeleteVertexArrays(1, &loader.vao);

    return 0;
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(error_callback);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "42run", nullptr, nullptr);
    if (window == nullptr)
    {
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    // stbi_set_flip_vertically_on_load(true);

    int ret = run(window);

    glfwTerminate();

    return ret;
}
