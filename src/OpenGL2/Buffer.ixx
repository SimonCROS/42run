module;
#include "glad/gl.h"

export module OpenGL.Buffer2;
export import :Builder;
import std;
import OpenGL.StateCache;

export namespace OpenGL
{
    class Buffer2
    {
    private:
        StateCache* m_stateCache;
        GLuint m_id;
        GLenum m_target;

    public:
        Buffer2() = delete;

        explicit Buffer2(std::nullptr_t) noexcept : m_stateCache(nullptr), m_id(0), m_target(0)
        {
        }

        explicit Buffer2(StateCache* stateCache, const GLuint id, const GLenum target) noexcept
            : m_stateCache(stateCache), m_id(id), m_target(target)
        {
        }

        Buffer2(const Buffer2&) = delete;
        auto operator=(const Buffer2&) -> Buffer2& = delete;

        Buffer2(Buffer2&& other) noexcept
            : m_stateCache(std::exchange(other.m_stateCache, nullptr))
              , m_id(std::exchange(other.m_id, 0))
              , m_target(std::exchange(other.m_target, 0))
        {
        }

        auto operator=(Buffer2&& other) noexcept -> Buffer2&
        {
            if (this != &other)
            {
                std::swap(m_stateCache, other.m_stateCache);
                std::swap(m_id, other.m_id);
                std::swap(m_target, other.m_target);
            }
            return *this;
        }

        ~Buffer2() noexcept
        {
            if (m_id != 0)
            {
                glDeleteBuffers(1, &m_id);
                m_stateCache->invalidateBuffer(m_target, m_id);
                m_id = 0;
                m_target = 0;
            }
        }

        auto bind() const -> void
        {
            if (m_stateCache->setBoundBuffer(m_target, m_id))
                glBindBuffer(m_target, m_id);
        }

        [[nodiscard]]
        static auto builder(StateCache* stateCache) noexcept -> BufferBuilder
        {
            return BufferBuilder(stateCache);
        }

        [[nodiscard]]
        constexpr auto id() const noexcept -> GLuint { return m_id; }
    };
}

export template <>
struct std::formatter<OpenGL::Buffer2>
{
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <class FormatContext>
    auto format(const OpenGL::Buffer2& obj, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        return std::format_to(ctx.out(),
                              "Buffer2{{id:{}}}",
                              obj.id());
    }
};
