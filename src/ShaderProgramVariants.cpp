#include "ShaderProgramVariants.hpp"

#include "Shader.hpp"

ShaderProgramVariants::ShaderProgramVariants(const std::string_view &vertPath, const std::string_view &fragPath)
{
    if (!Shader::TryGetShaderCode(vertPath, &_vertCode))
    {
        // std::string("Error while reading shader file ") + vertPath.data()
        throw std::exception(); // TODO exception
    }

    if (!Shader::TryGetShaderCode(fragPath, &_fragCode))
    {
        // std::string("Error while reading shader file ") + vertPath.data()
        throw std::exception(); // TODO exception
    }
}

void ShaderProgramVariants::Destroy()
{
    for (auto &program : programs)
    {
        program.second.Destroy();
    }
    programs.clear();
}

ShaderProgram &ShaderProgramVariants::GetProgram(ShaderFlags flags)
{
    auto it = programs.find(flags);
    if (it == programs.end())
    {
        throw std::exception(); // TODO exception
    }

    return it->second;
}

#include <iostream>

bool ShaderProgramVariants::EnableVariant(ShaderFlags flags)
{
    if (programs.count(flags) != 0)
    {
        return true;
    }

    std::string modifiedVertCode = GetCodeWithFlags(_vertCode, flags);
    std::string modifiedFragCode = GetCodeWithFlags(_fragCode, flags);

    ShaderProgram program(modifiedVertCode, modifiedFragCode);

    programs.emplace(flags, program);
    return program.id != 0;
}

bool ShaderProgramVariants::EnableVariants(const std::unordered_set<ShaderFlags> &variants)
{
    for (auto flags : variants)
    {
        if (!EnableVariant(flags))
        {
            return false;
        }
    }
    return true;
}

std::string ShaderProgramVariants::GetCodeWithFlags(const std::string &code, const ShaderFlags flags)
{
    std::string defines;
    if (flags & SHADER_HAS_NORMALS)
        defines += "#define HAS_NORMALS\n";
    if (flags & SHADER_HAS_TANGENTS)
        defines += "#define HAS_TANGENTS\n";
    if (flags & SHADER_HAS_BASECOLORMAP)
        defines += "#define HAS_BASECOLORMAP\n";
    if (flags & SHADER_HAS_METALROUGHNESSMAP)
        defines += "#define HAS_METALROUGHNESSMAP\n";
    if (flags & SHADER_HAS_NORMALMAP)
        defines += "#define HAS_NORMALMAP\n";
    if (flags & SHADER_HAS_EMISSIVEMAP)
        defines += "#define HAS_EMISSIVEMAP\n";

    if (defines.empty())
    {
        return code;
    }

    const std::size_t afterVersionIndex = 13;

    std::string copy = code;
    return copy.insert(afterVersionIndex, defines);
}

ShaderFlags GetPrimitiveShaderFlags(const tinygltf::Model &model, const tinygltf::Primitive &primitive)
{
    ShaderFlags primitiveShaderFlags = SHADER_HAS_NONE; // TODO store flags in primitive when loading to avoid recalculate
    for (const auto &[attribute, accessorId] : primitive.attributes)
    {
        if (attribute == "NORMAL")
        {
            primitiveShaderFlags |= SHADER_HAS_NORMALS;
        }
        else if (attribute == "TANGENT")
        {
            primitiveShaderFlags |= SHADER_HAS_TANGENTS;
        }
    }
    if (primitive.material >= 0)
    {
        const auto &material = model.materials[primitive.material];
        if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
            primitiveShaderFlags |= SHADER_HAS_BASECOLORMAP;
        if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
            primitiveShaderFlags |= SHADER_HAS_METALROUGHNESSMAP;
        if (material.normalTexture.index >= 0)
            primitiveShaderFlags |= SHADER_HAS_NORMALMAP;
        if (material.emissiveTexture.index >= 0)
            primitiveShaderFlags |= SHADER_HAS_EMISSIVEMAP;
    }

    return primitiveShaderFlags;
}
