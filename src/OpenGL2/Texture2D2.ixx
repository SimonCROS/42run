//
// Created by Simon Cros on 4/26/25.
//

module;
#include "glad/gl.h"

export module OpenGL.Texture2D2;
export import :Builder;
import std;
import glToString;
import OpenGL.StateCache;
import Image;

export namespace OpenGL
{
    class Texture2D2
    {
    private:
        StateCache * m_stateCache;
        GLuint m_id;
        GLint m_internalFormat;
        GLsizei m_width;
        GLsizei m_height;
        GLenum m_format;
        GLenum m_type;

    public:
        Texture2D2() = delete;

        explicit Texture2D2(StateCache * stateCache,
                            const GLuint id,
                            const GLint internalFormat,
                            const GLsizei width,
                            const GLsizei height,
                            const GLenum format,
                            const GLenum type) noexcept
            : m_stateCache(stateCache), m_id(id), m_internalFormat(internalFormat),
              m_width(width), m_height(height), m_format(format), m_type(type)
        {}

        Texture2D2(const Texture2D2 &) = delete;

        auto operator=(const Texture2D2 &) -> Texture2D2 & = delete;

        Texture2D2(Texture2D2 && other) noexcept
            : m_stateCache(std::exchange(other.m_stateCache, nullptr))
              , m_id(std::exchange(other.m_id, 0))
              , m_internalFormat(std::exchange(other.m_internalFormat, 0))
              , m_width(std::exchange(other.m_width, 0))
              , m_height(std::exchange(other.m_height, 0))
              , m_format(std::exchange(other.m_format, 0))
              , m_type(std::exchange(other.m_type, 0))
        {}

        auto operator=(Texture2D2 && other) noexcept -> Texture2D2 &
        {
            std::swap(m_stateCache, other.m_stateCache);
            std::swap(m_id, other.m_id);
            std::swap(m_internalFormat, other.m_internalFormat);
            std::swap(m_width, other.m_width);
            std::swap(m_height, other.m_height);
            std::swap(m_format, other.m_format);
            std::swap(m_type, other.m_type);
            return *this;
        }

        ~Texture2D2() noexcept
        {
            if (m_id != 0)
            {
                glDeleteTextures(1, &m_id);
                m_stateCache->invalidateBoundTexturesCache();
                m_id = 0;
            }
        }

        auto bind(const GLuint unit) const -> void
        {
            if (m_stateCache->setActiveTexture(unit))
                glActiveTexture(unit);
            if (m_stateCache->setBoundTexture(m_id))
                glBindTexture(GL_TEXTURE_2D, m_id);
        }

        [[nodiscard]]
        static auto builder(StateCache * stateCache) noexcept -> Texture2DBuilder
        {
            return Texture2DBuilder(stateCache);
        }

        [[nodiscard]]
        constexpr auto id() const noexcept -> GLuint { return m_id; }
        [[nodiscard]]
        constexpr auto internalFormat() const noexcept -> GLint { return m_internalFormat; }
        [[nodiscard]]
        constexpr auto width() const noexcept -> GLsizei { return m_width; }
        [[nodiscard]]
        constexpr auto height() const noexcept -> GLsizei { return m_height; }
        [[nodiscard]]
        constexpr auto format() const noexcept -> GLenum { return m_format; }
        [[nodiscard]]
        constexpr auto type() const noexcept -> GLenum { return m_type; }
    };
}

export template<>
struct std::formatter<OpenGL::Texture2D2>
{
    template<class ParseContext>
    constexpr auto parse(ParseContext & ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template<class FormatContext>
    auto format(const OpenGL::Texture2D2 & obj, FormatContext & ctx) const -> typename FormatContext::iterator
    {
        return std::format_to(ctx.out(),
                              "Texture2D2{{id:{},internalFormat:{},width:{},height:{},format:{},type:{}}}",
                              obj.id(),
                              glFormatToString(obj.internalFormat()),
                              obj.width(),
                              obj.height(),
                              glFormatToString(obj.format()),
                              glTypeToString(obj.type()));
    }
};
