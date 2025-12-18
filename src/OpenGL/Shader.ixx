//
// Created by scros on 11/29/25.
//

module;

#include <cassert>

#include "Utility/EnumHelpers.h"
#include "glad/gl.h"

export module Shader;
import std;
import ShaderFile;
import ShaderFlags;
import Utility.SlotSet;

export class Shader
{
public:
    SlotSetIndex index;

private:
    SlotSetIndex m_fileIdx;
    GLint m_id;
    GLenum m_type;
    ShaderFlags m_flags;

public:
    [[nodiscard]] static auto Create(const GLenum type,
                                     const SlotSetIndex fileIdx,
                                     const ShaderFlags flags) -> std::expected<Shader, std::string>
    {
        const GLuint id = glCreateShader(type);
        if (id == 0)
        {
            return std::unexpected("Failed to create new shader");
        }

        return std::expected<Shader, std::string>{std::in_place, type, fileIdx, flags, id};
    }

    explicit Shader(const GLenum type, const SlotSetIndex shaderFile, const ShaderFlags flags, const GLint id)
        : m_fileIdx(shaderFile), m_id(id), m_type(type), m_flags(flags)
    {}

    Shader(Shader && other) noexcept : index(std::exchange(other.index, {})),
                                       m_fileIdx(std::exchange(other.m_fileIdx, {})),
                                       m_id(std::exchange(other.m_id, {})),
                                       m_type(std::exchange(other.m_type, {})),
                                       m_flags(std::exchange(other.m_flags, {}))
    {}

    Shader(const Shader &) = delete;

    Shader & operator=(const Shader &) = delete;

    Shader & operator=(Shader && other) noexcept
    {
        std::swap(index, other.index);
        std::swap(m_fileIdx, other.m_fileIdx);
        std::swap(m_id, other.m_id);
        std::swap(m_type, other.m_type);
        std::swap(m_flags, other.m_flags);
        return *this;
    }

    ~Shader()
    {
        glDeleteShader(m_id);
    }

    [[nodiscard]] auto fileIdx() const -> SlotSetIndex { return m_fileIdx; }
    [[nodiscard]] auto flags() const -> ShaderFlags { return m_flags; }
    [[nodiscard]] auto type() const -> GLenum { return m_type; }
    [[nodiscard]] auto id() const -> GLuint { return m_id; }

    [[nodiscard]] auto update(const ShaderFile & file) -> std::expected<void, std::string>
    {
        assert(file.index == m_fileIdx && "Unexpected shader file");

        if (!file.isValid())
        {
            return std::unexpected("ShaderFile is invalid");
        }

        const auto code = file.createCodeForFlags(m_flags);
        const auto codePtr = code.data();
        const auto length = static_cast<GLint>(code.size());

        glShaderSource(m_id, 1, &codePtr, &length);
        glCompileShader(m_id);

        int success;
        glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[1024];
            GLsizei infoLength;
            glGetShaderInfoLog(m_id, 1024, &infoLength, infoLog);
            return std::unexpected(std::string(infoLog, infoLength));
        }

        return {};
    }
};
