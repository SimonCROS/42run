//
// Created by scros on 11/29/25.
//

export module ShaderManager;
import std;
import Shader;
import ShaderProgram;
import ShaderFile;
import Utility.SlotSet;

class ShaderManager
{
private:
    SlotSet<Shader> m_shaders;
    SlotSet<ShaderFile> m_shaderFiles;

public:
    [[nodiscard]] auto reloadAllShaders() -> std::expected<void, std::string>
    {
        for (auto & shaderFile : m_shaderFiles)
        {
            const auto && e_result = shaderFile.readCode();
            if (!e_result)
            {
                return e_result;
            }
        }

        for (const auto & shader : m_shaders)
        {
            const auto && e_result = m_shaderFiles[shader.index].compile(shader.flags, shader.id);
            if (!e_result)
            {
                return e_result;
            }
        }

        return {};
    }
};
