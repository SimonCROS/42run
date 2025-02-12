//
// Created by loumarti on 1/15/25.
//

#include "VertexBuffer.h"
#include "glad/gl.h"

VertexBuffer::VertexBuffer(): m_rendererID(0)
{
    glGenBuffers(1, &m_rendererID);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &m_rendererID);
}

auto VertexBuffer::bind() const -> void
{
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

auto VertexBuffer::unbind() const -> void
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

auto VertexBuffer::write(const void* data, unsigned int size) -> void
{
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
