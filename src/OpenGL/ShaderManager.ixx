//
// Created by scros on 11/29/25.
//

module;

#include <cassert>
#include "glad/gl.h"

export module ShaderManager;
import std;
import Shader;
import ShaderProgram;
import ShaderFile;
import ShaderFlags;
import Utility.SlotSet;

export struct ShaderProgramDefinition
{
    std::string_view vertexShaderFilePath;
    std::string_view fragmentShaderFilePath;
    SlotSetIndex vertexShaderFileIndex;
    SlotSetIndex fragmentShaderFileIndex;
    ShaderFlags shaderFlags; // Unified, but can be split into two variables for vertex/fragment
};

export class ShaderManager
{
private:
    SlotSet<ShaderProgram> m_shaderPrograms;
    SlotSet<Shader> m_shaders;
    SlotSet<ShaderFile> m_shaderFiles;

public:
    [[nodiscard]] auto addShaderFile(const std::string_view & path)
        -> std::expected<SlotSetIndex, std::string>
    {
        auto shaderFile = ShaderFile(path);
        if (const auto && e_result = shaderFile.readCode(); !e_result)
        {
            return std::unexpected(std::move(e_result).error());
        }

        return {m_shaderFiles.emplace(std::move(shaderFile)).index};
    }

    [[nodiscard]] auto createShader(const GLenum type, const SlotSetIndex fileIdx, const ShaderFlags flags)
        -> std::expected<SlotSetIndex, std::string>
    {
        assert(fileIdx != SlotSetIndex::invalid() && "file index is invalid");

        auto e_shader = Shader::Create(type, fileIdx, flags);
        if (!e_shader)
        {
            return std::unexpected(std::move(e_shader).error());
        }

        return {m_shaders.emplace(std::move(e_shader).value()).index};
    }

    [[nodiscard]] auto createShaderProgram(const SlotSetIndex vertexShaderIdx, const SlotSetIndex fragmentShaderIdx)
        -> std::expected<SlotSetIndex, std::string>
    {
        assert(vertexShaderIdx != SlotSetIndex::invalid() && "vertex shader index is invalid");
        assert(fragmentShaderIdx != SlotSetIndex::invalid() && "fragment shader index is invalid");

        auto e_shaderProgram = ShaderProgram::Create(m_shaders[vertexShaderIdx], m_shaders[fragmentShaderIdx]);
        if (!e_shaderProgram)
        {
            return std::unexpected(std::move(e_shaderProgram).error());
        }

        return {m_shaderPrograms.emplace(std::move(e_shaderProgram).value()).index};
    }

    [[nodiscard]] auto getOrAddShaderFile(const std::string_view & path)
        -> std::expected<SlotSetIndex, std::string>
    {
        for (const auto & shaderFile: m_shaderFiles)
        {
            if (shaderFile.path() == path)
            {
                return shaderFile.index;
            }
        }
        return addShaderFile(path);
    }

    [[nodiscard]] auto getOrCreateShader(const GLenum type, const SlotSetIndex fileIdx, const ShaderFlags flags)
        -> std::expected<SlotSetIndex, std::string>
    {
        assert(fileIdx != SlotSetIndex::invalid() && "file index is invalid");

        for (const auto & shader: m_shaders)
        {
            if (shader.type() == type && shader.fileIdx() == fileIdx && shader.flags() == flags)
            {
                return shader.index;
            }
        }
        return createShader(type, fileIdx, flags);
    }

    [[nodiscard]] auto getOrCreateShaderProgram(const SlotSetIndex vertexShaderIdx,
                                                const SlotSetIndex fragmentShaderIdx)
        -> std::expected<SlotSetIndex, std::string>
    {
        assert(vertexShaderIdx != SlotSetIndex::invalid() && "vertex shader index is invalid");
        assert(fragmentShaderIdx != SlotSetIndex::invalid() && "fragment shader index is invalid");

        for (const auto & program: m_shaderPrograms)
        {
            if (program.vertexShaderIdx() == vertexShaderIdx && program.fragmentShaderIdx() == fragmentShaderIdx)
            {
                return program.index;
            }
        }
        return createShaderProgram(vertexShaderIdx, fragmentShaderIdx);
    }

    [[nodiscard]] auto getOrCreateShaderProgram(const SlotSetIndex vertexShaderFileIdx,
                                                const SlotSetIndex fragmentShaderFileIdx,
                                                const ShaderFlags shaderFlags)
        -> std::expected<SlotSetIndex, std::string>
    {
        const auto e_vertexResult = getOrCreateShader(GL_VERTEX_SHADER,
                                                      vertexShaderFileIdx,
                                                      shaderFlags & ShaderFlags::HasSkin);
        if (!e_vertexResult)
        {
            return std::move(e_vertexResult);
        }

        const auto e_fragmentResult = getOrCreateShader(GL_FRAGMENT_SHADER,
                                                        fragmentShaderFileIdx,
                                                        shaderFlags & ~ShaderFlags::HasSkin);
        if (!e_fragmentResult)
        {
            return std::move(e_fragmentResult);
        }

        return getOrCreateShaderProgram(*e_vertexResult, *e_fragmentResult);
    }

    [[nodiscard]] auto reloadAllShaders() -> std::expected<void, std::string>
    {
        for (ShaderFile & shaderFile: m_shaderFiles)
        {
            if (const auto && e_result = shaderFile.readCode(); !e_result)
            {
                return e_result;
            }
        }

        for (Shader & shader: m_shaders)
        {
            if (const auto && e_result = compile(shader); !e_result)
            {
                return e_result;
            }
        }

        for (ShaderProgram & program: m_shaderPrograms)
        {
            if (const auto && e_result = link(program); !e_result)
            {
                return e_result;
            }
        }

        return {};
    }

    [[nodiscard]] auto compile(Shader & shader) const -> std::expected<void, std::string>
    {
        const ShaderFile & shaderFile = m_shaderFiles[shader.fileIdx()];
        return shader.update(shaderFile);
    }

    [[nodiscard]] auto link(ShaderProgram & shaderProgram) const -> std::expected<void, std::string>
    {
        const Shader & vertexShader = m_shaders[shaderProgram.vertexShaderIdx()];
        const Shader & fragmentShader = m_shaders[shaderProgram.fragmentShaderIdx()];
        return shaderProgram.link(vertexShader, fragmentShader);
    }
};
