//
// Created by scros on 11/29/25.
//

module;

#include "GL/gl.h"

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

export struct Shader
{
    SlotSetIndex index;
    SlotSetIndex shaderFileIdx;
    GLint id;
    ShaderFlags flags;
};
