#define TINYGLTF_IMPLEMENTATION

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
    ModelLoader::loadBinary(RESOURCE_PATH "magic_laboratory.glb", model);

    //! Generate buffers
    unsigned int VBO;
    unsigned int EBO;
    unsigned int VAO;
    {
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glGenVertexArrays(1, &VAO);
    }

    //! Bind buffers
    {
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    }

    //! Create shader program
    const ShaderProgram shaderProgram(
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

    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
    shaderProgram.use();
    shaderProgram.set_mat4("transform", trans);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        trans = glm::rotate(trans, glm::radians(1.0f), glm::vec3(0.0, 0.0, 1.0));

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        shaderProgram.use();
        shaderProgram.set_mat4("transform", trans);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    return 0;
}

int main(void)
{
    std::cout << "my directory is " << std::filesystem::current_path() << "\n";

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
