//
// Created by Simon Cros on 4/29/25.
//

module;
#include "glad/gl.h"

module OpenGL.Cubemap;
import glm;
import OpenGL;
import Engine;

namespace OpenGL
{
    auto CubemapBuilder::build() const -> std::expected<Cubemap, std::string>
    {
        GLuint id;
        glGenTextures(1, &id);

        if (id == 0)
        {
            return std::unexpected<std::string>("Failed to generate texture");
        }

        if (m_stateCache->setActiveTexture(GL_TEXTURE0))
        {
            glActiveTexture(GL_TEXTURE0);
        }

        if (m_stateCache->setBoundTexture(id))
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        }

        if (m_debugLabel != nullptr && glObjectLabel != nullptr)
        {
            glObjectLabel( GL_TEXTURE, id, static_cast<GLint>(std::strlen(m_debugLabel)), m_debugLabel);
        }

        for (GLint l = 0; l < 5; ++l)
        {
            for (GLuint i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             l,
                             m_internalFormat,
                             m_size >> l,
                             m_size >> l,
                             0,
                             GL_RGBA, // dummy
                             GL_UNSIGNED_BYTE, // dummy
                             nullptr);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, m_minFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, m_magFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 4);

        return std::expected<Cubemap, std::string>{std::in_place, m_stateCache, id, m_size};
    }

    auto Cubemap::fromEquirectangular(ShaderProgram & converter, const Texture2D& equirectangular) -> std::expected<void, std::string>
    {
        GLuint captureFBO;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

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

        for (unsigned int i = 0; i < 6; ++i)
        {
            converter.setMat4("u_projectionView", captureProjection * captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   m_id,
                                   0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &captureFBO);
        glEnable(GL_DEPTH_TEST);

        return {};
    }

    // TODO change to from shader because cubemap can be mapped outside the function
    auto Cubemap::fromCubemap(ShaderProgram & converter, const Cubemap& cubemap, const GLint level) -> std::expected<void, std::string>
    {
        GLuint captureFBO;

        glDisable(GL_DEPTH_TEST);
        glGenFramebuffers(1, &captureFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glViewport(0, 0, m_size >> level, m_size >> level);

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

        // ------------ TMP ------------
        float roughness = (float)level / (float)(5 - 1);
        converter.setFloat("u_roughness", roughness);
        // ------------ TMP ------------

        for (unsigned int i = 0; i < 6; ++i)
        {
            converter.setMat4("u_projectionView", captureProjection * captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   m_id,
                                   level);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &captureFBO);
        glEnable(GL_DEPTH_TEST);

        return {};
    }
}
