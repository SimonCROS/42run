//
// Created by scros on 11/29/25.
//

module;

#include "glad/gl.h"

export module ShaderManager;
import std;
import Shader;
import ShaderProgram;
import ShaderFile;
import ShaderFlags;
import Utility.SlotSet;

class ShaderManager
{
private:
    SlotSet<ShaderProgram> m_shaderPrograms;
    SlotSet<Shader> m_shaders;
    SlotSet<ShaderFile> m_shaderFiles;

public:
    [[nodiscard]] auto createShaderProgram(const SlotSetIndex vertexShaderIdx,
                                           const SlotSetIndex fragmentShaderIdx) -> std::expected<SlotSetIndex,
        std::string>
    {
        const auto e_shaderProgram = ShaderProgram::Create(m_shaders[vertexShaderIdx], m_shaders[fragmentShaderIdx]);
        if (!e_shaderProgram)
        {
            return std::unexpected(std::move(e_shaderProgram).error());
        }

        return {m_shaderPrograms.emplace(std::move(e_shaderProgram).value()).index};
    }

    [[nodiscard]] auto createShader(const GLenum type, const SlotSetIndex fileIdx,
                                    const ShaderFlags flags) -> std::expected<SlotSetIndex, std::string>
    {
        const auto e_shader = Shader::Create(type, fileIdx, flags);
        if (!e_shader)
        {
            return std::unexpected(std::move(e_shader).error());
        }

        return {m_shaders.emplace(std::move(e_shader).value()).index};
    }

    [[nodiscard]] auto reloadAllShaders() -> std::expected<void, std::string>
    {
        for (ShaderFile & shaderFile: m_shaderFiles)
        {
            const auto && e_result = shaderFile.readCode();
            if (!e_result)
            {
                return e_result;
            }
        }

        for (Shader & shader: m_shaders)
        {
            const auto && e_result = compile(shader);
            if (!e_result)
            {
                return e_result;
            }
        }

        for (ShaderProgram & program: m_shaderPrograms)
        {
            const auto && e_result = link(program);
            if (!e_result)
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
