//
// Created by Simon Cros on 4/26/25.
//

module;
#include "glad/gl.h"

export module OpenGL.Cubemap;
export import :Builder;
import std;
import glToString;
import OpenGL;
import OpenGL.StateCache;
import Image;
import OpenGL.Texture2D;

export namespace OpenGL
{
    class Cubemap
    {
    private:
        StateCache * m_stateCache;
        GLsizei m_size;
        GLuint m_id;
        GLuint m_baseLevel;
        GLuint m_maxLevel;

    public:
        Cubemap() = delete;

        explicit Cubemap(std::nullptr_t) noexcept : m_stateCache(nullptr), m_id(0), m_size(0)
        {}

        explicit Cubemap(StateCache * stateCache, const GLuint id, const GLsizei size, const GLuint baseLevel, const GLuint maxLevel) noexcept
            : m_stateCache(stateCache), m_size(size), m_id(id), m_baseLevel(baseLevel), m_maxLevel(maxLevel)
        {}

        Cubemap(const Cubemap &) = delete;

        auto operator=(const Cubemap &) -> Cubemap & = delete;

        Cubemap(Cubemap && other) noexcept
            : m_stateCache(std::exchange(other.m_stateCache, nullptr)),
              m_size(std::exchange(other.m_size, 0)),
              m_id(std::exchange(other.m_id, 0)),
              m_baseLevel(std::exchange(other.m_baseLevel, 0)),
              m_maxLevel(std::exchange(other.m_maxLevel, 0))
        {}

        auto operator=(Cubemap && other) noexcept -> Cubemap &
        {
            if (this != &other)
            {
                std::swap(m_stateCache, other.m_stateCache);
                std::swap(m_size, other.m_size);
                std::swap(m_id, other.m_id);
                std::swap(m_baseLevel, other.m_baseLevel);
                std::swap(m_maxLevel, other.m_maxLevel);
            }
            return *this;
        }

        ~Cubemap() noexcept
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
                glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
        }

        [[nodiscard]]
        static auto builder(StateCache * stateCache) noexcept -> CubemapBuilder
        {
            return CubemapBuilder(stateCache);
        }

        [[nodiscard]]
        auto fromCache(const std::filesystem::path & path, GLenum format, GLenum type) -> bool;
        [[nodiscard]]
        auto saveCache(const std::filesystem::path & path, GLenum format, GLenum type) const -> std::expected<void, std::string>;

        auto fromRaw(GLenum format, GLenum type, const void * pixels, GLint level, GLuint face) -> void;

        [[nodiscard]]
        auto fromEquirectangular(ShaderProgram & converter, const Texture2D & equirectangular)
            -> std::expected<void, std::string>;

        [[nodiscard]]
        auto fromCubemap(ShaderProgram & converter, const Cubemap & cubemap, GLint level)
            -> std::expected<void, std::string>;

        [[nodiscard]]
        constexpr auto id() const noexcept -> GLuint { return m_id; }

        [[nodiscard]]
        constexpr auto size() const noexcept -> GLsizei { return m_size; }
    };
}

export template<>
struct std::formatter<OpenGL::Cubemap>
{
    template<class ParseContext>
    constexpr auto parse(ParseContext & ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template<class FormatContext>
    auto format(const OpenGL::Cubemap & obj, FormatContext & ctx) const -> typename FormatContext::iterator
    {
        return std::format_to(ctx.out(),
                              "Cubemap{{id:{}}}",
                              obj.id());
    }
};
