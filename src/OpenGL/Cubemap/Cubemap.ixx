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
        StateCache* m_stateCache;
        GLuint m_id;
        GLsizei m_size;

    public:
        Cubemap() = delete;

        explicit Cubemap(std::nullptr_t) noexcept : m_stateCache(nullptr), m_id(0), m_size(0)
        {
        }

        explicit Cubemap(StateCache* stateCache, const GLuint id, const GLsizei size) noexcept
            : m_stateCache(stateCache), m_id(id), m_size(size)
        {
        }

        Cubemap(const Cubemap&) = delete;
        auto operator=(const Cubemap&) -> Cubemap& = delete;

        Cubemap(Cubemap&& other) noexcept
            : m_stateCache(std::exchange(other.m_stateCache, nullptr)),
              m_id(std::exchange(other.m_id, 0)),
              m_size(std::exchange(other.m_size, 0))
        {
        }

        auto operator=(Cubemap&& other) noexcept -> Cubemap&
        {
            if (this != &other)
            {
                std::swap(m_stateCache, other.m_stateCache);
                std::swap(m_id, other.m_id);
                std::swap(m_size, other.m_size);
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
        static auto builder(StateCache* stateCache) noexcept -> CubemapBuilder
        {
            return CubemapBuilder(stateCache);
        }

        [[nodiscard]]
        auto fromEquirectangular(ShaderProgram & converter, const Texture2D& equirectangular)
            -> std::expected<void, std::string>;

        [[nodiscard]]
        auto fromCubemap(ShaderProgram & converter, const Cubemap& cubemap, GLint level)
            -> std::expected<void, std::string>;

        [[nodiscard]]
        constexpr auto id() const noexcept -> GLuint { return m_id; }

        [[nodiscard]]
        constexpr auto size() const noexcept -> GLsizei { return m_size; }
    };
}

export template <>
struct std::formatter<OpenGL::Cubemap>
{
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <class FormatContext>
    auto format(const OpenGL::Cubemap& obj, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        return std::format_to(ctx.out(),
                              "Cubemap{{id:{}}}",
                              obj.id());
    }
};
