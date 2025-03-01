//
// Created by Simon Cros on 2/6/25.
//

module;

#include "glad/gl.h"

module OpenGL;

auto VertexArray::Create(VertexArrayFlags flags) -> VertexArray
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
    if (flags & VertexArrayHasTangents)
        glEnableVertexAttribArray(4);
    if (flags & VertexArrayHasSkin)
    {
        glEnableVertexAttribArray(5);
        glEnableVertexAttribArray(6);
    }

    return {flags, id};
}
