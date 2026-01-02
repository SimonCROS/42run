//
// Created by Simon Cros on 4/29/25.
//

module;
#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

module OpenGL.Cubemap2;
import OpenGL;
import Engine;

static constexpr GLfloat cubeStrip[] = {
    -0.5f, 0.5f, 0.5f,     // Front-top-left
    0.5f, 0.5f, 0.5f,      // Front-top-right
    -0.5f, -0.5f, 0.5f,    // Front-bottom-left
    0.5f, -0.5f, 0.5f,     // Front-bottom-right
    0.5f, -0.5f, -0.5f,    // Back-bottom-right
    0.5f, 0.5f, 0.5f,      // Front-top-right
    0.5f, 0.5f, -0.5f,     // Back-top-right
    -0.5f, 0.5f, 0.5f,     // Front-top-left
    -0.5f, 0.5f, -0.5f,    // Back-top-left
    -0.5f, -0.5f, 0.5f,    // Front-bottom-left
    -0.5f, -0.5f, -0.5f,   // Back-bottom-left
    0.5f, -0.5f, -0.5f,    // Back-bottom-right
    -0.5f, 0.5f, -0.5f,    // Back-top-left
    0.5f, 0.5f, -0.5f      // Back-top-right
};

namespace OpenGL
{
    auto CubemapBuilder::build() const -> std::expected<Cubemap2, std::string>
    {
        GLuint id;
        glGenTextures(1, &id);

        if (m_stateCache->setActiveTexture(GL_TEXTURE0))
            glActiveTexture(GL_TEXTURE0);
        if (m_stateCache->setBoundTexture(id))
            glBindTexture(GL_TEXTURE_CUBE_MAP, id);

        for (GLuint i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0,
                         m_internalFormat,
                         m_size,
                         m_size,
                         0,
                         m_format,
                         m_type,
                         m_faceData[i]);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_minFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_magFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return std::expected<Cubemap2, std::string>{std::in_place, m_stateCache, id, m_size};
    }

    auto Cubemap2::fromEquirectangular(ShaderProgram & converter, const Texture2D2& equirectangular) -> std::expected<void, std::string>
    {
        GLuint captureFBO;

        glDisable(GL_DEPTH_TEST);
        glGenFramebuffers(1, &captureFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glViewport(0, 0, m_size, m_size);

        const std::array<glm::mat4, 6> captureViews = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
        };

        const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

        glUseProgram(converter.id()); // Bad way to use
        converter.setInt("u_equirectangularMap", 0);

        equirectangular.bind(GL_TEXTURE0);

        // TODO optimize ? (unit_cube generic)
        GLuint cubeVao, cubeVbo;
        glGenVertexArrays(1, &cubeVao);
        glGenBuffers(1, &cubeVbo);
        glBindVertexArray(cubeVao);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeStrip), cubeStrip, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        for (unsigned int i = 0; i < 6; ++i)
        {
            converter.setMat4("u_projectionView", captureProjection * captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   m_id,
                                   0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, std::size(cubeStrip) / 3);
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glDeleteBuffers(1, &cubeVbo);
        glDeleteVertexArrays(1, &cubeVao);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &captureFBO);
        glEnable(GL_DEPTH_TEST);

        return {};
    }

    auto Cubemap2::fromCubemap(ShaderProgram & converter, const OpenGL::Cubemap2& cubemap) -> std::expected<void, std::string>
    {
        GLuint captureFBO;

        glDisable(GL_DEPTH_TEST);
        glGenFramebuffers(1, &captureFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glViewport(0, 0, m_size, m_size);

        const std::array<glm::mat4, 6> captureViews = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
        };

        const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

        glUseProgram(converter.id()); // Bad way to use
        converter.setInt("u_cubemap", 0);

        cubemap.bind(GL_TEXTURE0);

        // TODO optimize ? (unit_cube generic)
        GLuint cubeVao, cubeVbo;
        glGenVertexArrays(1, &cubeVao);
        glGenBuffers(1, &cubeVbo);
        glBindVertexArray(cubeVao);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeStrip), cubeStrip, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        for (unsigned int i = 0; i < 6; ++i)
        {
            converter.setMat4("u_projectionView", captureProjection * captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   m_id,
                                   0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, std::size(cubeStrip) / 3);
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glDeleteBuffers(1, &cubeVbo);
        glDeleteVertexArrays(1, &cubeVao);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &captureFBO);
        glEnable(GL_DEPTH_TEST);

        return {};
    }
}
