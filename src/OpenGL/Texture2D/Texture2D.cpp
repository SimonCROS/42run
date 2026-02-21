//
// Created by Simon Cros on 4/29/25.
//

module;
#include "glad/gl.h"

module OpenGL.Texture2D;
import ShaderProgram;
import Engine;

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

        return std::expected<Texture2D, std::string>{std::in_place, m_stateCache, id, m_internalFormat, m_width, m_height, m_format, m_type};
    }

    auto Texture2D::fromRaw()
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, m_format, m_type, nullptr);
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
