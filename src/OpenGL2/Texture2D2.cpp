//
// Created by Simon Cros on 4/29/25.
//

module;
#include "glad/gl.h"

module OpenGL.Texture2D2;

namespace OpenGL
{
    auto Texture2DBuilder::build() const -> std::expected<Texture2D2, std::string>
    {
        GLuint id;
        glGenTextures(1, &id);

        if (m_stateCache->setActiveTexture(0))
            glActiveTexture(0);
        if (m_stateCache->setBoundTexture(id))
            glBindTexture(GL_TEXTURE_2D, id);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     m_internalFormat,
                     m_width,
                     m_height,
                     0,
                     m_format,
                     m_type,
                     m_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);

        return std::expected<Texture2D2, std::string>{std::in_place, m_stateCache, id};
    }
}
