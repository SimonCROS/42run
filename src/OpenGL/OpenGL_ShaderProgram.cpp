//
// Created by Simon Cros on 3/1/25.
//

module;

#include <fstream>
#include <sstream>
#include <expected>

#include "42runConfig.h"
#include "tiny_gltf.h"

module OpenGL;

ShaderProgram::ShaderProgram(std::string&& vertCode, std::string&& fragCode)
    : m_vertCode(std::move(vertCode)), m_fragCode(std::move(fragCode))
{
}

auto ShaderProgram::Create(const std::string& vertPath, const std::string& fragPath)
    -> std::expected<ShaderProgram, std::string>
{
    auto e_vertCode = tryGetShaderCode(vertPath);
    if (!e_vertCode)
        return std::unexpected(std::move(e_vertCode).error());

    auto e_fragCode = tryGetShaderCode(fragPath);
    if (!e_fragCode)
        return std::unexpected(std::move(e_fragCode).error());

    return std::expected<ShaderProgram, std::string>{std::in_place, *std::move(e_vertCode), *std::move(e_fragCode)};
}

auto ShaderProgram::getProgram(const ShaderFlags flags) -> ShaderProgramInstance&
{
    const auto it = programs.find(flags);
    if (it == programs.end())
    {
        throw std::exception(); // TODO exception
    }

    return *it->second;
}

auto ShaderProgram::getProgram(const ShaderFlags flags) const -> const ShaderProgramInstance&
{
    const auto it = programs.find(flags);
    if (it == programs.end())
    {
        throw std::exception(); // TODO exception
    }

    return *it->second;
}

auto ShaderProgram::enableVariant(const ShaderFlags flags)
    -> std::expected<std::reference_wrapper<ShaderProgramInstance>, std::string>
{
    {
        const auto it = programs.find(flags);
        if (it != programs.end())
            return *it->second;
    }

    const std::string modifiedVertCode = getCodeWithFlags(m_vertCode, flags);
    const std::string modifiedFragCode = getCodeWithFlags(m_fragCode, flags);

    auto e_program = ShaderProgramInstance::Create(modifiedVertCode, modifiedFragCode);
    if (!e_program)
        return std::unexpected(std::move(e_program).error());

    auto [it, inserted] = programs.try_emplace(flags, std::make_unique<ShaderProgramInstance>(*std::move(e_program)));

    if (!inserted)
        return std::unexpected("A shader with the same id already exist");
    return *it->second;
}

#define QUDI(x) #x
#define QUdi(x) QUDI(x)
auto ShaderProgram::getCodeWithFlags(const std::string_view& code, const ShaderFlags flags) -> std::string
{
    constexpr std::size_t afterVersionIndex = 13;

    std::string defines;
    defines += "#define MAX_JOINTS " QUdi(MAX_JOINTS) "\n";
    if (flags & ShaderHasNormals)
        defines += "#define HAS_NORMALS\n";
    if (flags & ShaderHasTangents)
        defines += "#define HAS_TANGENTS\n";
    if (flags & ShaderHasBaseColorMap)
        defines += "#define HAS_BASECOLORMAP\n";
    if (flags & ShaderHasMetalRoughnessMap)
        defines += "#define HAS_METALROUGHNESSMAP\n";
    if (flags & ShaderHasNormalMap)
        defines += "#define HAS_NORMALMAP\n";
    if (flags & ShaderHasEmissiveMap)
        defines += "#define HAS_EMISSIVEMAP\n";
    if (flags & ShaderHasVec3Colors)
        defines += "#define HAS_VEC3_COLORS\n";
    if (flags & ShaderHasVec4Colors)
        defines += "#define HAS_VEC4_COLORS\n";
    if (flags & ShaderHasSkin)
        defines += "#define HAS_SKIN\n";
    if (flags & ShaderHasTexCoord0)
        defines += "#define HAS_TEXCOORD_0\n";
    if (flags & ShaderHasTexCoord1)
        defines += "#define HAS_TEXCOORD_1\n";

    auto copy = std::string(code);
    if (defines.empty())
        return copy;

    copy.insert(afterVersionIndex, defines);
    return copy;
}

auto ShaderProgram::tryGetShaderCode(const std::string& path) -> std::expected<std::string, std::string>
{
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        std::stringstream stream;
        file.open(path.c_str());
        stream << file.rdbuf();
        file.close();

        return std::move(stream).str();
    }
    catch (std::ifstream::failure& e)
    {
        return std::unexpected("Failed to get shader code at `" + path + "`: " + e.what());
    }
}

auto GetPrimitiveShaderFlags(const tinygltf::Model& model, const tinygltf::Primitive& primitive) -> ShaderFlags
{
    ShaderFlags primitiveShaderFlags = ShaderHasNone; // TODO store flags in primitive when loading to avoid recalculate
    for (const auto& [attribute, accessorId] : primitive.attributes)
    {
        if (attribute == "NORMAL")
            primitiveShaderFlags |= ShaderHasNormals;
        else if (attribute == "TANGENT")
            primitiveShaderFlags |= ShaderHasTangents;
        else if (attribute == "COLOR_0" && model.accessors[accessorId].type == TINYGLTF_TYPE_VEC3)
            primitiveShaderFlags |= ShaderHasVec3Colors;
        else if (attribute == "COLOR_0" && model.accessors[accessorId].type == TINYGLTF_TYPE_VEC4)
            primitiveShaderFlags |= ShaderHasVec4Colors;
    }
    if (primitive.material >= 0)
    {
        const auto& material = model.materials[primitive.material];
        if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
            primitiveShaderFlags |= ShaderHasBaseColorMap;
        if (material.normalTexture.index >= 0)
            primitiveShaderFlags |= ShaderHasNormalMap;
    }

    return primitiveShaderFlags;
}
