//
// Created by Simon Cros on 4/29/25.
//

module;
#include <cstdio>

#include "glad/gl.h"

module OpenGL.Cubemap;
import DataCache;
import glm;
import OpenGL;
import OpenGL.Utility;
import Engine;

static auto textureSizeInMemoryForLevel(const GLenum format,
                                        const GLenum type,
                                        const uint32_t baseWidth,
                                        const uint32_t baseHeight,
                                        const uint32_t level) -> uint32_t
{
    const uint32_t pixelSize = OpenGL::formatComponentsCount(format) * OpenGL::typeSize(type);
    const uint32_t levelsSize = (baseWidth * baseHeight) >> level;
    return pixelSize * levelsSize;
}

static auto textureSizeInMemory(const GLenum format,
                                const GLenum type,
                                const uint32_t baseWidth,
                                const uint32_t baseHeight,
                                const uint32_t baseLevel,
                                const uint32_t maxLevel) -> uint32_t
{
    const uint32_t pixelSize = OpenGL::formatComponentsCount(format) * OpenGL::typeSize(type);
    uint32_t levelsSize = 0;
    for (uint32_t level = baseLevel; level <= maxLevel; ++level)
    {
        levelsSize += (baseWidth * baseHeight) >> level;
    }
    return pixelSize * levelsSize;
}

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

        // if (m_debugLabel != nullptr)
        // {
        //     glObjectLabel(GL_TEXTURE, id, static_cast<GLint>(std::strlen(m_debugLabel)), m_debugLabel);
        // }

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
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, m_baseLevel);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, m_maxLevel);

        return std::expected<Cubemap, std::string>{std::in_place, m_stateCache, id, m_size, m_baseLevel, m_maxLevel};
    }

    auto Cubemap::fromCache(const std::filesystem::path & path, const GLenum format, const GLenum type) -> bool
    {
        const auto oe_result = DataCache::readFile(path);

        if (!oe_result)
        {
            return false;
        }

        if (!oe_result->has_value())
        {
            std::println(stderr, "Failed to load texture from {}: {}", path.c_str(), oe_result->error());
            return false;
        }

        const uint32_t saveSize = textureSizeInMemory(format, type, size(), size(), m_baseLevel, m_maxLevel) * 6;

        if (oe_result->value().size() != saveSize)
        {
            std::println(stderr, "Failed to load texture from {}: unexpected size", path.c_str(),
                         oe_result->value().size());
            return false;
        }

        uint32_t offset = 0;
        for (GLuint level = m_baseLevel; level <= m_maxLevel; ++level)
        {
            for (GLuint face = 0; face < 6; ++face)
            {
                fromRaw(format, type, oe_result->value().data() + offset, level, face);
                offset += textureSizeInMemoryForLevel(format, type, size(), size(), level);
            }
        }
        return true;
    }

    auto Cubemap::saveCache(const std::filesystem::path & path, const GLenum format,
                            const GLenum type) const -> std::expected<void, std::string>
    {
        const uint32_t saveSize = textureSizeInMemory(format, type, size(), size(), m_baseLevel, m_maxLevel) * 6;

        std::vector<std::byte> pixels(saveSize);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

        uint32_t offset = 0;
        for (GLuint level = m_baseLevel; level <= m_maxLevel; ++level)
        {
            for (GLuint face = 0; face < 6; ++face)
            {
                glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, format, type, pixels.data() + offset);
                offset += textureSizeInMemoryForLevel(format, type, size(), size(), level);
            }
        }

        TRY(DataCache::writeFile(path, pixels));
        return {};
    }

    auto Cubemap::fromRaw(const GLenum format, const GLenum type, const void * const pixels,
                          const GLint level, const GLuint face) -> void
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0, m_size >> level, m_size >> level, format, type, pixels);
    }

    auto Cubemap::fromEquirectangular(ShaderProgram & converter,
                                      const Texture2D & equirectangular) -> std::expected<void, std::string>
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
    auto Cubemap::fromCubemap(ShaderProgram & converter, const Cubemap & cubemap,
                              const GLint level) -> std::expected<void, std::string>
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
        float roughness = (float) level / (float) (5 - 1);
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
