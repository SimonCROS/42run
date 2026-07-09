//
// Created by Simon Cros on 4/29/25.
//

module;
#include "glad/gl.h"
#include <cstdio>

module OpenGL.Cubemap;
import std;
import glm;
import OpenGL;
import Engine;
import DataCache;
import OpenGL.Utility;

namespace OpenGL
{
    auto createCubemap(StateCache & stateCache, const CubemapCreateInfo & info) -> std::expected<Cubemap, std::string>
    {
        GLuint id;
        glGenTextures(1, &id);

        if (id == 0)
        {
            return std::unexpected<std::string>("Failed to generate texture");
        }

        if (stateCache.setBoundTexture(id))
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, id);
        }

        if (info.debugLabel != nullptr && glObjectLabel != nullptr)
        {
            glObjectLabel(GL_TEXTURE, id, static_cast<GLint>(std::strlen(info.debugLabel)), info.debugLabel);
        }

        for (GLint level = info.baseLevel; level <= info.maxLevel; ++level)
        {
            for (GLuint faceIdx = 0; faceIdx < 6; ++faceIdx)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx,
                             level,
                             info.internalFormat,
                             info.size >> level,
                             info.size >> level,
                             0,
                             GL_RGBA, // dummy
                             GL_UNSIGNED_BYTE, // dummy
                             nullptr);
            }
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, info.minFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, info.magFilter);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, info.baseLevel);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, info.maxLevel);

        return std::expected<Cubemap, std::string>{std::in_place, &stateCache, id, info};
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

        const uint32_t pixelSize = formatComponentsCount(format) * typeSize(type);

        uint32_t totalSize = 0;
        for (int32_t level = m_info.baseLevel; level <= m_info.maxLevel; ++level)
        {
            const uint32_t levelSize = m_info.size >> level;
            totalSize += levelSize * levelSize * 6 * pixelSize;
        }

        if ((*oe_result)->size() != totalSize)
        {
            std::println(stderr, "Failed to load texture from {}: invalid size", path.c_str());
            return false;
        }

        std::vector<std::byte> pixels(totalSize);

        bind(GL_TEXTURE0);

        uint32_t offset = 0;
        for (GLint level = m_info.baseLevel; level <= m_info.maxLevel; ++level)
        {
            for (GLuint faceIdx = 0; faceIdx < 6; ++faceIdx)
            {
                fromRaw(format, type, (*oe_result)->data() + offset, level, faceIdx);

                const uint32_t levelSize = m_info.size >> level;
                offset += levelSize * levelSize * pixelSize;
            }
        }
        return true;
    }

    auto Cubemap::saveCache(const std::filesystem::path & path, const GLenum format, const GLenum type) const -> std::expected<void, std::string>
    {
        const uint32_t pixelSize = formatComponentsCount(format) * typeSize(type);

        uint32_t totalSize = 0;
        for (int32_t level = m_info.baseLevel; level <= m_info.maxLevel; ++level)
        {
            const uint32_t levelSize = m_info.size >> level;
            totalSize += levelSize * levelSize * 6 * pixelSize;
        }

        std::vector<std::byte> pixels(totalSize);
        bind(GL_TEXTURE0);

        uint32_t offset = 0;
        for (GLint level = m_info.baseLevel; level <= m_info.maxLevel; ++level)
        {
            for (GLuint faceIdx = 0; faceIdx < 6; ++faceIdx)
            {
                glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceIdx, level, format, type, pixels.data() + offset);

                const uint32_t levelSize = m_info.size >> level;
                offset += levelSize * levelSize * pixelSize;
            }
        }

        TRY(DataCache::writeFile(path, pixels));
        return {};
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    auto Cubemap::fromRaw(const GLenum format, const GLenum type, const void * const pixels,
                          const GLint level, const GLuint face) -> void
    {
        bind(GL_TEXTURE0);
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, level, 0, 0, m_info.size >> level, m_info.size >> level, format, type, pixels);
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    auto Cubemap::fromShader(ShaderProgram & program, const GLint level) -> std::expected<void, std::string>
    {
        GLuint captureFBO;

        glDisable(GL_DEPTH_TEST);
        glGenFramebuffers(1, &captureFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glViewport(0, 0, m_info.size >> level, m_info.size >> level);

        const std::array<glm::mat4, 6> captureViews = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
        };

        const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

        for (unsigned int i = 0; i < 6; ++i)
        {
            program.setMat4("u_projectionView", captureProjection * captureViews[i]);
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

    // ReSharper disable once CppMemberFunctionMayBeConst
    auto Cubemap::generateMipmap() -> void
    {
        bind(GL_TEXTURE0);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
}
