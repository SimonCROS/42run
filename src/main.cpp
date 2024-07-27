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

static void CheckErrors(std::string desc)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR)
    {
        fprintf(stderr, "OpenGL error in \"%s\": %d (%d)\n", desc.c_str(), e, e);
        exit(20);
    }
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

constexpr static void *BufferOffset(const long offset)
{
    return reinterpret_cast<void *>(offset);
}

static GLuint GetDrawMode(int tinygltfMode)
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

static void DrawMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const std::map<int, GLuint> buffers, const ShaderProgram &program)
{
    for (const auto &primitive : mesh.primitives)
    {
        if (primitive.indices < 0)
            continue;

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

                glEnableVertexAttribArray(attributeLocation);
                CheckErrors("enable vertex attrib array");
            }
        }

        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.at(indexAccessor.bufferView));
        CheckErrors("bind buffer");

        int mode = GetDrawMode(primitive.mode);
        assert(mode != -1);

        glDrawElements(mode, indexAccessor.count, indexAccessor.componentType,
                       BufferOffset(indexAccessor.byteOffset));
        CheckErrors("draw elements");

        for (const auto &[attribute, accessorId] : primitive.attributes)
        {
            int attributeLocation = program.GetAttributeLocation(attribute);
            if (attributeLocation != -1)
            {
                glDisableVertexAttribArray(attributeLocation);
            }
        }
    }
}

static void DrawNode(tinygltf::Model &model, const tinygltf::Node &node, const std::map<int, GLuint> buffers, const ShaderProgram &program, glm::dmat4 transform)
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
        DrawMesh(model, model.meshes[node.mesh], buffers, program);
    }
    for (const int &child : node.children)
    {
        DrawNode(model, model.nodes[child], buffers, program, transform);
    }
}

static int run(GLFWwindow *window)
{
    int version = gladLoadGL(glfwGetProcAddress);
    std::cout << "OpenGL " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;

    float vertices[] = {
        // positions      // colors         // texture coords
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };

    unsigned int indices[] = {
        // note that we start from 0!
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    tinygltf::Model model;
    GLuint vao;
    std::map<int, GLuint> buffers;
    ModelLoader::loadBinary(RESOURCE_PATH "magic_laboratory.glb", &model, &vao, buffers);

    //! Create shader program
    const ShaderProgram program(
        Shader(RESOURCE_PATH "shaders/default.vert", GL_VERTEX_SHADER),
        Shader(RESOURCE_PATH "shaders/default.frag", GL_FRAGMENT_SHADER));

    //! Create texture
    unsigned int texture;
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        stbi_uc *data = stbi_load(RESOURCE_PATH "textures/uvs.png", &width, &height, &nrChannels, 0);

        if (data == nullptr)
        {
            std::cerr << "ERROR::TEXTURE::LOADING_FAILED\n"
                      << stbi_failure_reason() << std::endl;
            stbi_image_free(data);
            return 1;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glm::dmat4 transform = glm::identity<glm::dmat4>();
    transform = glm::rotate(transform, glm::radians(45.0), glm::dvec3(0.0, 1.0, 0.0));

    glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 5.2f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 1.0f, 0.0f);   
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);

        program.Use();
        program.SetMat4("projection", proj);
        program.SetMat4("view", view);
        for (const auto &scene : model.scenes)
        {
            for (const int &child : scene.nodes)
            {
                DrawNode(model, model.nodes[child], buffers, program, transform);
            }
        }

        // for (const auto &mesh : model.meshes)
        // {
        //     DrawMesh(model, mesh, buffers, program);
        // }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    return 0;
}

int main(void)
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(error_callback);
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "42run", NULL, NULL);
    if (window == nullptr)
    {
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    stbi_set_flip_vertically_on_load(true);

    int ret = run(window);

    glfwTerminate();

    return ret;
}
