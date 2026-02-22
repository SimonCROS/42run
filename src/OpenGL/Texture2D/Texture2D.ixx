//
// Created by Simon Cros on 4/26/25.
//

module;
#include "glad/gl.h"

export module OpenGL.Texture2D;
export import :Builder;
import std;
import glToString;
import OpenGL.StateCache;
import Image;
import ShaderProgram;

export namespace OpenGL
{
    class Texture2D
    {
    private:
        StateCache * m_stateCache;
        GLuint m_id;
        GLint m_internalFormat;
        GLsizei m_width;
        GLsizei m_height;

    public:
        Texture2D() = delete;

        explicit Texture2D(StateCache * stateCache,
                            const GLuint id,
                            const GLint internalFormat,
                            const GLsizei width,
                            const GLsizei height) noexcept
            : m_stateCache(stateCache), m_id(id), m_internalFormat(internalFormat),
              m_width(width), m_height(height)
        {}

        Texture2D(const Texture2D &) = delete;

        auto operator=(const Texture2D &) -> Texture2D & = delete;

        Texture2D(Texture2D && other) noexcept
            : m_stateCache(std::exchange(other.m_stateCache, nullptr))
              , m_id(std::exchange(other.m_id, 0))
              , m_internalFormat(std::exchange(other.m_internalFormat, 0))
              , m_width(std::exchange(other.m_width, 0))
              , m_height(std::exchange(other.m_height, 0))
        {}

        auto operator=(Texture2D && other) noexcept -> Texture2D &
        {
            std::swap(m_stateCache, other.m_stateCache);
            std::swap(m_id, other.m_id);
            std::swap(m_internalFormat, other.m_internalFormat);
            std::swap(m_width, other.m_width);
            std::swap(m_height, other.m_height);
            return *this;
        }

        ~Texture2D() noexcept
        {
            if (m_id != 0)
            {
                glDeleteTextures(1, &m_id);
                m_stateCache->invalidateBoundTexturesCache();
                m_id = 0;
            }
        }

        static auto serialize(const Texture2D & texture) -> std::vector<std::byte>
        {
            // std::vector<unsigned char> pixels(texture.width() * texture.height() * );
            // glBindTexture(GL_TEXTURE_2D, textureID);
            // glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

            return {};
        }

        static auto deserialize(const std::vector<std::byte> & data) -> std::expected<Texture2D, std::string>
        {
            return std::unexpected<std::string>("a");
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
        auto fromCache(const std::filesystem::path & path, GLenum format, GLenum type) -> bool;
        auto saveCache(const std::filesystem::path & path, GLenum format, GLenum type) const -> bool; // NOLINT(modernize-use-nodiscard)

        auto fromRaw(GLenum format, GLenum type, const void * pixels) -> void;

        [[nodiscard]]
        auto fromShader(ShaderProgram & converter) -> std::expected<void, std::string>;
    };
}

export template<>
struct std::formatter<OpenGL::Texture2D>
{
    template<class ParseContext>
    constexpr auto parse(ParseContext & ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template<class FormatContext>
    auto format(const OpenGL::Texture2D & obj, FormatContext & ctx) const -> FormatContext::iterator
    {
        return std::format_to(ctx.out(),
                              "Texture2D{{id:{},internalFormat:{},width:{},height:{}}}",
                              obj.id(),
                              glFormatToString(obj.internalFormat()),
                              obj.width(),
                              obj.height());
    }
};
