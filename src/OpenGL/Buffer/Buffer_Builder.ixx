module;
#include "glad/gl.h"

export module OpenGL.Buffer:Builder;
import std;
import OpenGL.StateCache;

export namespace OpenGL
{
    class Buffer;

    class BufferBuilder
    {
    private:
        StateCache* m_stateCache;
        GLenum m_target;
        GLsizeiptr m_size;
        const void* m_data;
        GLenum m_usage;

    public:
        explicit BufferBuilder(StateCache* stateCache) noexcept
            : m_stateCache(stateCache)
            , m_target(GL_ARRAY_BUFFER)
            , m_size(0)
            , m_data(nullptr)
            , m_usage(GL_STATIC_DRAW)
        {
        }

        [[nodiscard]]
        auto withTarget(const GLenum target) noexcept -> BufferBuilder&
        {
            m_target = target;
            return *this;
        }

        [[nodiscard]]
        auto withData(const GLsizeiptr size, const void* data) noexcept -> BufferBuilder&
        {
            m_size = size;
            m_data = data;
            return *this;
        }

        [[nodiscard]]
        auto withUsage(const GLenum usage) noexcept -> BufferBuilder&
        {
            m_usage = usage;
            return *this;
        }

        [[nodiscard]]
        auto build() const -> std::expected<Buffer, std::string>;
    };
}
