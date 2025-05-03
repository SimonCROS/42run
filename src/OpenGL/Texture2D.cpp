//
// Created by Simon Cros on 21/03/2025.
//

module;
#include "glad/gl.h"
#include "stb_image.h"

module OpenGL;
import std;

typedef void* (*ImageLoadFunc)(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);

static auto loadTexture2D8u(const std::string& path) -> unsigned int
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                     data);
    }
    else
    {
        // TODO Create Image wrapper and free all images automatically. Also return expected<>
        std::println(stderr, "Texture2D tex failed to load at path: {}", path);
    }
    stbi_image_free(data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

static auto loadTexture2D32f(const std::string& path) -> unsigned int
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;

    float* data = stbi_loadf(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    }
    else
    {
        // TODO Create Image wrapper and free all images automatically. Also return expected<>
        std::println(stderr, "Texture2D tex failed to load at path: {}", path);
    }
    stbi_image_free(data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

Texture2D::Texture2D(Texture2D&& other) noexcept: m_textureId(std::exchange(other.m_textureId, 0))
{
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &m_textureId);
}

auto Texture2D::Create8u(const std::string& path) -> Texture2D
{
    return Texture2D(loadTexture2D8u(path));
}

auto Texture2D::Create32f(const std::string& path) -> Texture2D
{
    return Texture2D(loadTexture2D32f(path));
}
