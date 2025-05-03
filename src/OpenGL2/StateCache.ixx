//
// Created by Simon Cros on 4/26/25.
//

module;
#include "glad/gl.h"

export module OpenGL.StateCache;
import std;

export namespace OpenGL
{
    class StateCache
    {
    public:
        constexpr auto setActiveTexture(const GLuint unit) noexcept -> bool
        {
            if (m_activeTextureUnit != unit)
            {
                m_activeTextureUnit = unit;
                return true;
            }
            return false;
        }

        constexpr auto setBoundTexture(const GLuint texture) noexcept -> bool
        {
            if (m_boundTextures[m_activeTextureUnit] != texture)
            {
                m_boundTextures[m_activeTextureUnit] = texture;
                return true;
            }
            return false;
        }

        constexpr auto setBoundBuffer(const GLenum target, const GLuint buffer) noexcept -> bool
        {
            if (const auto it = m_boundBuffers.find(target); it->second != buffer)
            {
                it->second = buffer;
                return true;
            }
            return false;
        }

        constexpr auto invalidateBoundTexturesCache() noexcept -> void
        {
            std::ranges::fill(m_boundTextures, 0);
        }

        constexpr auto invalidateBuffer(const GLenum target, const GLuint buffer) noexcept -> void
        {
            if (const auto it = m_boundBuffers.find(target); it->second == buffer)
            {
                it->second = 0;
            }
        }

        [[nodiscard]] constexpr auto activeTextureUnit() const noexcept -> GLuint { return m_activeTextureUnit; }

        [[nodiscard]] constexpr auto boundTexture(const GLuint unit) const noexcept -> GLuint
        {
            return m_boundTextures[unit];
        }

    private:
        GLenum m_activeTextureUnit = 0;
        GLuint m_boundTextures[GL_MAX_TEXTURE_UNITS] = {0};
        GLuint m_boundFramebuffer = 0;
        std::unordered_map<GLenum, GLuint> m_boundBuffers{
            {GL_ARRAY_BUFFER, 0},
            // {GL_ATOMIC_COUNTER_BUFFER, 0},
            {GL_COPY_READ_BUFFER, 0},
            {GL_COPY_WRITE_BUFFER, 0},
            // {GL_DISPATCH_INDIRECT_BUFFER, 0},
            {GL_DRAW_INDIRECT_BUFFER, 0},
            {GL_ELEMENT_ARRAY_BUFFER, 0},
            {GL_PIXEL_PACK_BUFFER, 0},
            {GL_PIXEL_UNPACK_BUFFER, 0},
            // {GL_QUERY_BUFFER, 0},
            // {GL_SHADER_STORAGE_BUFFER, 0},
            {GL_TEXTURE_BUFFER, 0},
            {GL_TRANSFORM_FEEDBACK_BUFFER, 0},
            {GL_UNIFORM_BUFFER, 0},
        };
    };
}
