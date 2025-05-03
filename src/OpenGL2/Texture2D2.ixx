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
import OpenGL.Image;

export namespace OpenGL
{
    class Texture2D2
    {
    private:
        StateCache* m_stateCache;
        GLuint m_id;

    public:
        Texture2D2() = delete;

        explicit Texture2D2(std::nullptr_t) noexcept : m_stateCache(nullptr), m_id(0)
        {
        }

        explicit Texture2D2(StateCache* stateCache, const GLuint id) noexcept
            : m_stateCache(stateCache), m_id(id)
        {
        }

        Texture2D2(const Texture2D2&) = delete;
        auto operator=(const Texture2D2&) -> Texture2D2& = delete;

        Texture2D2(Texture2D2&& other) noexcept
            : m_stateCache(std::exchange(other.m_stateCache, nullptr))
            , m_id(std::exchange(other.m_id, 0))
        {
        }

        auto operator=(Texture2D2&& other) noexcept -> Texture2D2&
        {
            if (this != &other)
            {
                std::swap(m_stateCache, other.m_stateCache);
                std::swap(m_id, other.m_id);
            }
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
        static auto builder(StateCache* stateCache) noexcept -> Texture2DBuilder
        {
            return Texture2DBuilder(stateCache);
        }

        [[nodiscard]]
        constexpr auto id() const noexcept -> GLuint { return m_id; }
    };
}

export template <>
struct std::formatter<OpenGL::Texture2D2>
{
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <class FormatContext>
    auto format(const OpenGL::Texture2D2& obj, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        return std::format_to(ctx.out(),
                              "Texture2D2{{id:{}}}",
                              obj.id());
    }
};
