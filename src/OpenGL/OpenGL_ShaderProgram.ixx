//
// Created by Simon Cros on 3/1/25.
//

module;

#include "Utility/EnumHelpers.h"

export module OpenGL:ShaderProgram;
import std;
import :ShaderProgramInstance;
import Utility;

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

export
{
    MAKE_FLAG_ENUM(ShaderFlags)
}

export class ShaderProgram
{
public:
    std::unordered_map<ShaderFlags, std::unique_ptr<ShaderProgramInstance>> programs;

    ShaderProgram() = delete;
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&& other) = default; // TODO add destructor, so mark other as destroyed
    ShaderProgram(std::string&& vertCode, std::string&& fragCode);

    static auto Create(const std::string& vertPath,
                       const std::string& fragPath) -> std::expected<ShaderProgram, std::string>;

    auto getProgram(ShaderFlags flags) -> ShaderProgramInstance&;
    auto getProgram(ShaderFlags flags) const -> const ShaderProgramInstance&;
    auto enableVariant(ShaderFlags flags) -> std::expected<std::reference_wrapper<ShaderProgramInstance>, std::string>;

private:
    std::string m_vertCode;
    std::string m_fragCode;

    static auto getCodeWithFlags(const std::string_view& code, ShaderFlags flags) -> std::string;
    static auto tryGetShaderCode(const std::string& path) -> std::expected<std::string, std::string>;
};
