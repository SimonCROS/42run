//
// Created by scros on 12/7/25.
//

module;

#include "Utility/EnumHelpers.h"

export module ShaderFlags;

export enum class ShaderFlags : unsigned short
{
    HasNone = 0,
    HasNormals = 1 << 0,
    HasTangents = 1 << 1,
    HasBaseColorMap = 1 << 2,
    HasMetalRoughnessMap = 1 << 3,
    HasNormalMap = 1 << 4,
    HasEmissiveMap = 1 << 5,
    HasVec3Colors = 1 << 6,
    HasVec4Colors = 1 << 7,
    HasSkin = 1 << 8,
    HasTexCoord0 = 1 << 9,
    HasTexCoord1 = 1 << 10,
};

export
{
    MAKE_FLAG_ENUM(ShaderFlags);
}
