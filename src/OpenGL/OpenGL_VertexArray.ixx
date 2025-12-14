//
// Created by Simon Cros on 2/6/25.
//

module;

#include "glad/gl.h"
#include "Utility/EnumHelpers.h"

export module OpenGL:VertexArray;
import std;
import Utility;

export enum VertexArrayFlags : unsigned char
{
    VertexArrayHasNone = 0,
    VertexArrayHasPosition = 1 << 0,
    VertexArrayHasNormals = 1 << 1,
    VertexArrayHasColor0 = 1 << 2,
    VertexArrayHasTexCoord0 = 1 << 3,
    VertexArrayHasTexCoord1 = 1 << 4,
    VertexArrayHasTangents = 1 << 5,
    VertexArrayHasSkin = 1 << 6,
    VertexArrayHasWeights0 = 1 << 7,
};

export
{
    MAKE_FLAG_ENUM(VertexArrayFlags)
}

export class VertexArray
{
private:
    VertexArrayFlags m_flags{VertexArrayHasNone};
    GLuint m_id{0};

public:
    static auto Create(VertexArrayFlags flags) -> VertexArray
    {
        GLuint id;

        glGenVertexArrays(1, &id);
        glBindVertexArray(id);

        if (flags & VertexArrayHasPosition)
            glEnableVertexAttribArray(0);
        if (flags & VertexArrayHasNormals)
            glEnableVertexAttribArray(1);
        if (flags & VertexArrayHasColor0)
            glEnableVertexAttribArray(2);
        if (flags & VertexArrayHasTexCoord0)
            glEnableVertexAttribArray(3);
        if (flags & VertexArrayHasTexCoord1)
            glEnableVertexAttribArray(4);
        if (flags & VertexArrayHasTangents)
            glEnableVertexAttribArray(5);
        if (flags & VertexArrayHasSkin)
        {
            glEnableVertexAttribArray(6);
            glEnableVertexAttribArray(7);
        }

        return {flags, id};
    }

    VertexArray() = default;

    VertexArray(const VertexArrayFlags flags, const GLuint id) : m_flags(flags), m_id(id)
    {
    }

    VertexArray(const VertexArray&) = delete;

    VertexArray(VertexArray&& other) noexcept : m_flags(other.m_flags), m_id(std::exchange(other.m_id, 0))
    {
    }

    ~VertexArray()
    {
        glDeleteVertexArrays(1, &m_id);
    }

    auto operator=(const VertexArray&) -> VertexArray& = delete;

    auto operator=(VertexArray&& other) noexcept -> VertexArray&
    {
        m_flags = other.m_flags;
        std::swap(m_id, other.m_id);
        return *this;
    }

    auto bind() const -> void
    {
        glBindVertexArray(m_id);
    }

    [[nodiscard]] auto id() const -> GLuint { return m_id; }
};
