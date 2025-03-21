//
// Created by Simon Cros on 21/03/2025.
//

module;
#include "glad/gl.h"
#include "stb_image.h"

export module OpenGL:Cubemap;
import std;

export class Cubemap
{
private:
    GLuint m_textureId;

    static auto loadCubemap(const std::vector<std::string>& faces) -> unsigned int
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int width, height, nrChannels;
        for (unsigned int i = 0; i < faces.size(); i++)
        {
            unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                );
            }
            else
            {
                // TODO Create Image wrapper and free all images automatically. Also return expected<>
                std::println(stderr, "Cubemap tex failed to load at path: {}", faces[i]);
            }
            stbi_image_free(data);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return textureID;
    }

    explicit Cubemap(GLuint id) : m_textureId(id)
    {
    }

public:
    Cubemap(const Cubemap&) = delete;
    ~Cubemap()
    {
        glDeleteTextures(1, &m_textureId);
    }

    auto operator=(const Cubemap&) -> Cubemap& = delete;

    static auto Create(const std::vector<std::string>& faces) -> Cubemap
    {
        return Cubemap(loadCubemap(faces));
    }
};
