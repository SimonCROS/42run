//
// Created by Simon Cros on 2/6/25.
//

#include "VertexArray.h"

auto VertexArray::Create(VertexArrayFlags flags) -> VertexArray
{
    GLuint id;

    glGenVertexArrays(1, &id);
    glBindVertexArray(id);

    if (flags & VertexArrayHasPosition)
        glEnableVertexAttribArray(0);
    if (flags & VertexArrayHasNormal)
        glEnableVertexAttribArray(1);
    if (flags & VertexArrayHasColor0)
        glEnableVertexAttribArray(2);
    if (flags & VertexArrayHasTexCoord0)
        glEnableVertexAttribArray(3);

    return {flags, id};
}
