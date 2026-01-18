//
// Created by Simon Cros on 4/29/25.
//

module;
#include "glad/gl.h"

module OpenGL.Buffer;

namespace OpenGL
{
    auto BufferBuilder::build() const -> std::expected<Buffer, std::string>
    {
        GLuint id;
        glGenBuffers(1, &id);

        if (m_stateCache->setBoundBuffer(m_target, id))
            glBindBuffer(m_target, id);

        glBufferData(m_target, m_size, m_data, m_usage);

        return std::expected<Buffer, std::string>{std::in_place, m_stateCache, id, m_target};
    }
}
