#ifndef SHADER_PROGRAM_VARIANTS_H
#define SHADER_PROGRAM_VARIANTS_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <string_view>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "ShaderProgram.hpp"

enum ShaderFlags : unsigned char
{
    SHADER_HAS_NONE = 0,
    SHADER_HAS_NORMALS = 1 << 0,
    SHADER_HAS_TANGENTS = 1 << 1,
    SHADER_HAS_BASECOLORMAP = 1 << 2,
    SHADER_HAS_METALROUGHNESSMAP = 1 << 3,
    SHADER_HAS_NORMALMAP = 1 << 4,
    SHADER_HAS_EMISSIVEMAP = 1 << 5,
};

inline ShaderFlags operator~(ShaderFlags a) { return (ShaderFlags) ~(int)a; }
inline ShaderFlags operator|(ShaderFlags a, ShaderFlags b) { return (ShaderFlags)((int)a | (int)b); }
inline ShaderFlags operator&(ShaderFlags a, ShaderFlags b) { return (ShaderFlags)((int)a & (int)b); }
inline ShaderFlags operator^(ShaderFlags a, ShaderFlags b) { return (ShaderFlags)((int)a ^ (int)b); }
inline ShaderFlags &operator|=(ShaderFlags &a, ShaderFlags b) { return (ShaderFlags &)((int &)a |= (int)b); }
inline ShaderFlags &operator&=(ShaderFlags &a, ShaderFlags b) { return (ShaderFlags &)((int &)a &= (int)b); }
inline ShaderFlags &operator^=(ShaderFlags &a, ShaderFlags b) { return (ShaderFlags &)((int &)a ^= (int)b); }

class ShaderProgramVariants
{
public:
    std::unordered_map<ShaderFlags, ShaderProgram> programs;

    ShaderProgramVariants(const std::string_view &vertPath, const std::string_view &fragPath);

    void Destroy();

    ShaderProgram &GetProgram(ShaderFlags flags);
    bool EnableVariant(ShaderFlags flags);
    bool EnableVariants(const std::unordered_set<ShaderFlags>& flags);

private:
    ShaderProgramVariants() = default;
    ShaderProgramVariants(const ShaderProgramVariants &) = delete;

    std::string _vertCode;
    std::string _fragCode;

    static std::string GetCodeWithFlags(const std::string &code, const ShaderFlags flags);
};

ShaderFlags GetPrimitiveShaderFlags(const tinygltf::Model &model, const tinygltf::Primitive &primitive);

#endif
