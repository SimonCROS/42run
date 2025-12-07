//
// Created by scros on 11/29/25.
//

module;

#include "glad/gl.h"

export module ShaderFile;
import std;
import Shader;
import Utility.SlotSet;

#define QUDI(x) #x
#define QUdi(x) QUDI(x)

export class ShaderFile
{
public:
    SlotSetIndex index;

private:
    std::string m_path;
    std::string m_code;

public:
    [[nodiscard]] auto readCode() -> std::expected<void, std::string>
    {
        std::ifstream file;

        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            std::stringstream stream;
            file.open(m_path.c_str());
            stream << file.rdbuf();
            file.close();

            m_code = std::move(stream).str();
        }
        catch (std::ifstream::failure& e)
        {
            return std::unexpected("Failed to get shader code at `" + m_path + "`: " + e.what());
        }

        return {};
    }

    [[nodiscard]] auto code() const -> const std::string & { return m_code; }

    [[nodiscard]] auto createCodeForFlags(const ShaderFlags flags) const -> std::string
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

        auto copy = m_code;
        if (defines.empty())
            return copy;

        copy.insert(afterVersionIndex, defines);
        return copy;
    }

    [[nodiscard]] auto isValid() const -> bool { return !m_code.empty(); }
};
