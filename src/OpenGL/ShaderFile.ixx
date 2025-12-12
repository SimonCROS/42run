//
// Created by scros on 11/29/25.
//

export module ShaderFile;
import std;
import ShaderFlags;
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
    explicit ShaderFile(const std::string_view & path) : m_path(path) {}

    ShaderFile(ShaderFile && other) noexcept : index(std::exchange(other.index, {})),
                                                     m_path(std::exchange(other.m_path, {})),
                                                     m_code(std::exchange(other.m_code, {}))
    {}

    ShaderFile(const ShaderFile &) = delete;

    ShaderFile & operator=(const ShaderFile &) = delete;

    ShaderFile & operator=(ShaderFile && other) noexcept
    {
        std::swap(index, other.index);
        std::swap(m_path, other.m_path);
        std::swap(m_code, other.m_code);
        return *this;
    }

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
        catch (std::ifstream::failure & e)
        {
            return std::unexpected("Failed to get shader code at `" + m_path + "`: " + e.what());
        }

        return {};
    }

    [[nodiscard]] auto isValid() const -> bool { return !m_code.empty(); }

    [[nodiscard]] auto code() const -> const std::string & { return m_code; }

    [[nodiscard]] auto createCodeForFlags(const ShaderFlags flags) const -> std::string
    {
        constexpr std::size_t afterVersionIndex = 13;

        std::string defines;
        defines += "#define MAX_JOINTS " QUdi(MAX_JOINTS) "\n";
        if ((flags & ShaderFlags::HasNormals) == ShaderFlags::HasNormals)
            defines += "#define HAS_NORMALS\n";
        if ((flags & ShaderFlags::HasTangents) == ShaderFlags::HasTangents)
            defines += "#define HAS_TANGENTS\n";
        if ((flags & ShaderFlags::HasBaseColorMap) == ShaderFlags::HasBaseColorMap)
            defines += "#define HAS_BASECOLORMAP\n";
        if ((flags & ShaderFlags::HasMetalRoughnessMap) == ShaderFlags::HasMetalRoughnessMap)
            defines += "#define HAS_METALROUGHNESSMAP\n";
        if ((flags & ShaderFlags::HasNormalMap) == ShaderFlags::HasNormalMap)
            defines += "#define HAS_NORMALMAP\n";
        if ((flags & ShaderFlags::HasEmissiveMap) == ShaderFlags::HasEmissiveMap)
            defines += "#define HAS_EMISSIVEMAP\n";
        if ((flags & ShaderFlags::HasVec3Colors) == ShaderFlags::HasVec3Colors)
            defines += "#define HAS_VEC3_COLORS\n";
        if ((flags & ShaderFlags::HasVec4Colors) == ShaderFlags::HasVec4Colors)
            defines += "#define HAS_VEC4_COLORS\n";
        if ((flags & ShaderFlags::HasSkin) == ShaderFlags::HasSkin)
            defines += "#define HAS_SKIN\n";
        if ((flags & ShaderFlags::HasTexCoord0) == ShaderFlags::HasTexCoord0)
            defines += "#define HAS_TEXCOORD_0\n";
        if ((flags & ShaderFlags::HasTexCoord1) == ShaderFlags::HasTexCoord1)
            defines += "#define HAS_TEXCOORD_1\n";

        auto copy = m_code;
        if (defines.empty())
            return copy;

        copy.insert(afterVersionIndex, defines);
        return copy;
    }
};
