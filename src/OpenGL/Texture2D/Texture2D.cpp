//
// Created by Simon Cros on 4/29/25.
//

module;
#include <cstdio>

#include "glad/gl.h"

module OpenGL.Texture2D;
import DataCache;
import Engine;
import OpenGL.Utility;
import ShaderProgram;

namespace OpenGL
{
    auto Texture2DBuilder::build() const -> std::expected<Texture2D, std::string>
    {
        GLuint id;
        glGenTextures(1, &id);

        if (id == 0)
        {
            return std::unexpected<std::string>("Failed to generate texture");
        }

        if (m_stateCache->setActiveTexture(0))
        {
            glActiveTexture(GL_TEXTURE0);
        }

        if (m_stateCache->setBoundTexture(id))
        {
            glBindTexture(GL_TEXTURE_2D, id);
        }

        if (m_debugLabel != nullptr && glObjectLabel != nullptr)
        {
            glObjectLabel( GL_TEXTURE, id, static_cast<GLint>(std::strlen(m_debugLabel)), m_debugLabel);
        }

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     m_internalFormat,
                     m_width,
                     m_height,
                     0,
                     GL_RGBA, // dummy
                     GL_UNSIGNED_BYTE, // dummy
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);

        return std::expected<Texture2D, std::string>{
            std::in_place, m_stateCache, id, m_internalFormat, m_width, m_height
        };
    }

    auto Texture2D::fromCache(const std::filesystem::path & path, const GLenum format, const GLenum type) -> bool
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

        fromRaw(format, type, oe_result->value().data()); // TODO maybe return result of from data
        return true;
    }

    auto Texture2D::saveCache(const std::filesystem::path & path, const GLenum format, const GLenum type) const -> std::expected<void, std::string>
    {
        const uint32_t pixelSize = formatComponentsCount(format) * typeSize(type);

        std::vector<std::byte> pixels(width() * height() * pixelSize);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glGetTexImage(GL_TEXTURE_2D, 0, format, type, pixels.data());

        TRY(DataCache::writeFile(path, pixels));
        return {};
    }

    auto Texture2D::fromRaw(const GLenum format, const GLenum type, const void * const pixels) -> void
    {
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, format, type, pixels);
    }

    auto Texture2D::fromShader(ShaderProgram & converter) -> std::expected<void, std::string>
    {
        GLuint captureFBO;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_id);

        glDisable(GL_DEPTH_TEST);
        glGenFramebuffers(1, &captureFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glViewport(0, 0, m_width, m_height);

        glUseProgram(converter.id()); // Bad way to use

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               m_id,
                               0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderQuad();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &captureFBO);
        glEnable(GL_DEPTH_TEST);

        return {};
    }
}
