//
// Created by scros on 12/7/25.
//

module;

#include "Utility/EnumHelpers.h"

export module ShaderFlags;

export enum class ShaderFlags : unsigned short
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

export
{
    MAKE_FLAG_ENUM(ShaderFlags);
}
