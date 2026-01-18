//
// Created by scros on 12/7/25.
//

module;

#include "Utility/EnumHelpers.h"

export module ShaderFlags;

export enum class ShaderFlags : unsigned char
{
    None = 0,
    HasBaseColorMap = 1 << 1,
    HasMetalRoughnessMap = 1 << 2,
    HasNormalMap = 1 << 3,
    HasEmissiveMap = 1 << 4,
    HasSkin = 1 << 5,
};

export
{
    MAKE_FLAG_ENUM(ShaderFlags);
}
