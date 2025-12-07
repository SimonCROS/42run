//
// Created by scros on 11/29/25.
//

module;

#include "glad/gl.h"

export module Shader;
import Utility.SlotSet;

export enum ShaderFlags : unsigned short
{
    ShaderHasNone = 0,
    ShaderHasNormals = 1 << 0,
    ShaderHasTangents = 1 << 1,
    ShaderHasBaseColorMap = 1 << 2,
    ShaderHasMetalRoughnessMap = 1 << 3,
    ShaderHasNormalMap = 1 << 4,
    ShaderHasEmissiveMap = 1 << 5,
    ShaderHasVec3Colors = 1 << 6,
    ShaderHasVec4Colors = 1 << 7,
    ShaderHasSkin = 1 << 8,
    ShaderHasTexCoord0 = 1 << 9,
    ShaderHasTexCoord1 = 1 << 10,
};

export class Shader
{
public:
    SlotSetIndex index;

private:
    SlotSetIndex m_shaderFileIdx;
    GLint m_id;
    GLenum m_type;
    ShaderFlags m_flags;

public:
    Shader(const GLenum type, const SlotSetIndex shaderFile, const ShaderFlags flags, const GLint id)
        : m_shaderFileIdx(shaderFile), m_id(id), m_type(type), m_flags(flags)
    {}

    [[nodiscard]] static auto Create(const GLenum type,
                       const SlotSetIndex shaderFile,
                       const ShaderFlags flags) -> std::expected<Shader, std::string>
    {
        const GLuint id = glCreateShader(type);
        if (id == 0)
            return std::unexpected("Failed to create new shader id");

        return std::expected<Shader, std::string>{std::in_place, type, shaderFile, flags, id};
    }

    [[nodiscard]] auto fileIdx() const -> SlotSetIndex { return m_shaderFileIdx; }
    [[nodiscard]] auto flags() const -> ShaderFlags { return m_flags; }
    [[nodiscard]] auto type() const -> GLenum { return m_type; }
    [[nodiscard]] auto id() const -> GLuint { return m_id; }

    [[nodiscard]] auto update(const std::string_view & code) -> std::expected<void, std::string>
    {
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
