//
// Created by scros on 11/29/25.
//

module;

#include <cassert>

#include "glad/gl.h"

export module ShaderProgram;
import Utility.SlotSet;
import Shader;

export class ShaderProgram
{
public:
    SlotSetIndex index;

private:
    SlotSetIndex m_vertexShaderIdx;
    SlotSetIndex m_fragmentShaderIdx;
    GLint m_id;

public:
    [[nodiscard]] static auto Create(const Shader & vertexShader,
                                     const Shader & fragmentShader) -> std::expected<ShaderProgram, std::string>
    {
        const GLuint id = glCreateProgram();
        if (id == 0)
        {
            return std::unexpected("Failed to create new program");
        }

        glAttachShader(id, vertexShader.id());
        glAttachShader(id, fragmentShader.id());

        return std::expected<ShaderProgram, std::string>{std::in_place, vertexShader.index, fragmentShader.index, id};
    }

    explicit ShaderProgram(const SlotSetIndex vertexShaderIdx, const SlotSetIndex fragmentShaderIdx, const GLint id)
        : m_vertexShaderIdx(vertexShaderIdx), m_fragmentShaderIdx(fragmentShaderIdx), m_id(id)
    {}

    ShaderProgram(const ShaderProgram && other) noexcept : index(std::exchange(other.index, {})),
                                                           m_vertexShaderIdx(
                                                               std::exchange(other.m_vertexShaderIdx, {})),
                                                           m_fragmentShaderIdx(
                                                               std::exchange(other.m_fragmentShaderIdx, {})),
                                                           m_id(std::exchange(other.m_id, {}))
    {}

    ShaderProgram(const ShaderProgram &) = delete;

    ShaderProgram & operator=(const ShaderProgram &) = delete;

    ShaderProgram & operator=(const ShaderProgram &&) = delete;

    ~ShaderProgram()
    {
        glDeleteProgram(m_id);
    }

    [[nodiscard]] auto vertexShaderIdx() const -> SlotSetIndex { return m_vertexShaderIdx; }
    [[nodiscard]] auto fragmentShaderIdx() const -> SlotSetIndex { return m_fragmentShaderIdx; }
    [[nodiscard]] auto id() const -> GLuint { return m_id; }

    [[nodiscard]] auto link(const Shader & vertexShader,
                            const Shader & fragmentShader) -> std::expected<void, std::string>
    {
        assert(vertexShader.index == m_vertexShaderIdx && "Unexpected vertex shader");
        assert(fragmentShader.index == m_fragmentShaderIdx && "Unexpected fragment shader");

        int success;

        glLinkProgram(m_id);
        glGetProgramiv(m_id, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[1024];
            GLsizei infoLength;
            glGetProgramInfoLog(m_id, 1024, &infoLength, infoLog);
            return std::unexpected(std::string(infoLog, infoLength));
        }

        return {};
    }
};
