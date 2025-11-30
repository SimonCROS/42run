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
private:
    SlotSetIndex m_index;
    SlotSetIndex m_shaderFileIdx;
    GLint m_id;
    GLenum m_type;
    ShaderFlags m_flags;

public:
    Shader(const GLenum type, const SlotSetIndex shaderFile, const ShaderFlags flags, const GLint id)
        : m_shaderFileIdx(shaderFile), m_id(id), m_type(type), m_flags(flags)
    {}

    static auto Create(const GLenum type,
                       const SlotSetIndex shaderFile,
                       const ShaderFlags flags) -> std::expected<Shader, std::string>
    {
        const GLuint id = glCreateShader(type);
        if (id == 0)
            return std::unexpected("Failed to create new shader id");

        return std::expected<Shader, std::string>{std::in_place, type, shaderFile, flags, id};
    }
};
