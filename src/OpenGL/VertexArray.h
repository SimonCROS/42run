//
// Created by Simon Cros on 2/6/25.
//

#ifndef VAO_H
#define VAO_H
#include <unordered_map>
#include <glad/gl.h>

#include "../Utility/StringUnorderedMap.h"
#include "Utility/EnumHelpers.h"

enum VertexArrayFlags : unsigned char
{
    VertexArrayHasNone = 0,
    VertexArrayHasPosition = 1 << 0,
    VertexArrayHasNormals = 1 << 1,
    VertexArrayHasColor0 = 1 << 2,
    VertexArrayHasTexCoord0 = 1 << 3,
    VertexArrayHasTangents = 1 << 4,
    VertexArrayHasSkin = 1 << 4,
};

MAKE_FLAG_ENUM(VertexArrayFlags)

class VertexArray
{
private:
    VertexArrayFlags m_flags{VertexArrayHasNone};
    GLuint m_id{0};
    GLuint m_currentlyBoundArrayBuffer{0};
    GLuint m_currentlyBoundArrayElementBuffer{0};

    static inline const StringUnorderedMap<int> attributeLocations{
        {"POSITION", 0},
        {"NORMAL", 1},
        {"COLOR_0", 2},
        {"TEXCOORD_0", 3},
        {"TANGENT", 4},
        {"JOINTS_0", 5},
        {"WEIGHTS_0", 6},
    };

public:
    static auto Create(VertexArrayFlags flags) -> VertexArray;

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

    auto bindArrayBuffer(const GLuint id) -> void
    {
        if (m_currentlyBoundArrayBuffer != id)
        {
            glBindBuffer(GL_ARRAY_BUFFER, id);
            m_currentlyBoundArrayBuffer = id;
        }
    }

    auto bindElementArrayBuffer(const GLuint id) -> void
    {
        if (m_currentlyBoundArrayElementBuffer != id)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
            m_currentlyBoundArrayElementBuffer = id;
        }
    }

    [[nodiscard]] auto id() const -> GLuint { return m_id; }

    [[nodiscard]] static auto getAttributeLocation(const std::string_view& attribute) -> GLint
    {
        const auto it = attributeLocations.find(attribute);
        return (it != attributeLocations.cend()) ? it->second : -1;
    }
};


#endif //VAO_H
