//
// Created by loumarti on 1/15/25.
//

module;

#include "glad/gl.h"

module OpenGL;

IndicesBuffer::IndicesBuffer(): m_rendererID(0)
{
    glGenBuffers(1, &m_rendererID);
}

IndicesBuffer::~IndicesBuffer()
{
    glDeleteBuffers(1, &m_rendererID);
}

auto IndicesBuffer::bind() const -> void
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
}

auto IndicesBuffer::unbind() const -> void
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

auto IndicesBuffer::write(const void* data, unsigned int size) -> void
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
