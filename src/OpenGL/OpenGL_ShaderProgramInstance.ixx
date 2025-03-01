//
// Created by Simon Cros on 3/1/25.
//

module;

#include <string>
#include <unordered_map>
#include <expected>

#include "glad/gl.h"
#include "glm/glm.hpp"

export module OpenGL:ShaderProgramInstance;
import :Shader;
import Utility;

export class ShaderProgramInstance
{
    GLuint m_id;
    Shader m_vertShader;
    Shader m_fragShader;

public:
    static auto Create(const std::string_view& vertexCode, const std::string_view& fragCode)
        -> std::expected<ShaderProgramInstance, std::string>;

    ShaderProgramInstance() : m_id(0)
    {
    }

    ShaderProgramInstance(GLuint id, Shader&& vertShader, Shader&& fragShader);

    ShaderProgramInstance(const ShaderProgramInstance&) = delete;

    ShaderProgramInstance(ShaderProgramInstance&& other) noexcept
        : m_id(std::exchange(other.m_id, 0)),
          m_vertShader(std::exchange(other.m_vertShader, {})),
          m_fragShader(std::exchange(other.m_fragShader, {})),
          m_bools(std::exchange(other.m_bools, {})),
          m_ints(std::exchange(other.m_ints, {})),
          m_uints(std::exchange(other.m_uints, {})),
          m_floats(std::exchange(other.m_floats, {})),
          m_vec3s(std::exchange(other.m_vec3s, {})),
          m_vec4s(std::exchange(other.m_vec4s, {})),
          m_mat4s(std::exchange(other.m_mat4s, {}))
    {
    }

    ~ShaderProgramInstance()
    {
        glDeleteProgram(m_id);
    }

    auto operator=(const ShaderProgramInstance&) -> ShaderProgramInstance& = delete;

    auto operator=(ShaderProgramInstance&& other) noexcept -> ShaderProgramInstance&
    {
        std::swap(m_id, other.m_id);
        std::swap(m_vertShader, other.m_vertShader);
        std::swap(m_fragShader, other.m_fragShader);
        std::swap(m_bools, other.m_bools);
        std::swap(m_ints, other.m_ints);
        std::swap(m_uints, other.m_uints);
        std::swap(m_floats, other.m_floats);
        std::swap(m_vec3s, other.m_vec3s);
        std::swap(m_vec4s, other.m_vec4s);
        std::swap(m_mat4s, other.m_mat4s);
        return *this;
    }

    [[nodiscard]] auto id() const -> GLuint { return m_id; }

    void use() const;

    void setBool(const std::string_view& name, bool value);
    void setInt(const std::string_view& name, GLint value);
    void setUint(const std::string_view& name, GLuint value);
    void setFloat(const std::string_view& name, float value);
    void setVec3(const std::string_view& name, const glm::vec3& value);
    void setVec4(const std::string_view& name, const glm::vec4& value);
    void setMat4(const std::string_view& name, const glm::mat4& value);
    void setUniformBlock(const std::string_view& name, GLuint uniformBlockBinding);

private:
    StringUnorderedMap<bool> m_bools;
    StringUnorderedMap<GLint> m_ints;
    StringUnorderedMap<GLuint> m_uints;
    StringUnorderedMap<GLuint> m_uniformBlocks;
    StringUnorderedMap<float> m_floats;
    StringUnorderedMap<glm::vec3> m_vec3s;
    StringUnorderedMap<glm::vec4> m_vec4s;
    StringUnorderedMap<glm::mat4> m_mat4s;

    static auto linkProgram(GLuint id) -> std::expected<void, std::string>;

    template <typename T>
    static bool storeUniformValue(const std::string_view& name, T value, StringUnorderedMap<T>& map,
                                  const std::string** nullTerminatedString)
    {
        const auto it = map.find(name);
        if (it == map.end())
        {
            const auto inserted = map.emplace(std::string(name), value);
            *nullTerminatedString = &inserted.first->first;
            return true;
        }

        *nullTerminatedString = &it->first;
        if (it->second != value)
        {
            it->second = value;
            return true;
        }
        return false;
    }
};
