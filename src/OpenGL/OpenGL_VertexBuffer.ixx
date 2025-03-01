//
// Created by loumarti on 1/15/25.
//

module;

#include "glad/gl.h"

export module OpenGL:VertexBuffer;

export class VertexBuffer
{
private:
    unsigned int m_rendererID;

public:
    VertexBuffer(): m_rendererID(0)
    {
        glGenBuffers(1, &m_rendererID);
    }

    ~VertexBuffer()
    {
        glDeleteBuffers(1, &m_rendererID);
    }

    void bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    }

    void unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void write(const void* data, unsigned int size)
    {
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }

    [[nodiscard]] unsigned int getID() const { return m_rendererID; }
};
