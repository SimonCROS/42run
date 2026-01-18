//
// Created by loumarti on 1/15/25.
//

module;

#include "glad/gl.h"

export module OpenGL:IndicesBuffer;
import std;

export class IndicesBuffer
{
private:
    unsigned int m_rendererID;

public:
    IndicesBuffer(): m_rendererID(0)
    {
        glGenBuffers(1, &m_rendererID);
    }

    ~IndicesBuffer()
    {
        glDeleteBuffers(1, &m_rendererID);
    }

    auto bind() const -> void
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
    }

    auto unbind() const -> void
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    auto write(const void* data, const unsigned int size) -> void
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }

    [[nodiscard]] unsigned int getID() const
    {
        return m_rendererID;
    }
};
