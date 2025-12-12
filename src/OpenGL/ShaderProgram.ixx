//
// Created by scros on 11/29/25.
//

module;

#include <version>
#include <cassert>

#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

export module ShaderProgram;
import std;
import Shader;
import UniformValue;
import Utility.SlotSet;
import Utility.StringUnorderedMap;

export class ShaderProgram
{
public:
    SlotSetIndex index;

private:
    StringUnorderedMap<UniformValue> m_uniformCache;
    SlotSetIndex m_vertexShaderIdx;
    SlotSetIndex m_fragmentShaderIdx;
    GLint m_id;

public:
    [[nodiscard]] static auto Create(const Shader & vertexShader,
                                     const Shader & fragmentShader) -> std::expected<ShaderProgram, std::string>
    {
        const GLuint id = glCreateProgram();
        if (id == 0)
        {
            return std::unexpected("Failed to create new program");
        }

        glAttachShader(id, vertexShader.id());
        glAttachShader(id, fragmentShader.id());

        return std::expected<ShaderProgram, std::string>{std::in_place, vertexShader.index, fragmentShader.index, id};
    }

    explicit ShaderProgram(const SlotSetIndex vertexShaderIdx, const SlotSetIndex fragmentShaderIdx, const GLint id)
        : m_vertexShaderIdx(vertexShaderIdx), m_fragmentShaderIdx(fragmentShaderIdx), m_id(id)
    {}

    ShaderProgram(const ShaderProgram && other) noexcept : index(std::exchange(other.index, {})),
                                                           m_uniformCache(std::exchange(other.m_uniformCache, {})),
                                                           m_vertexShaderIdx(
                                                               std::exchange(other.m_vertexShaderIdx, {})),
                                                           m_fragmentShaderIdx(
                                                               std::exchange(other.m_fragmentShaderIdx, {})),
                                                           m_id(std::exchange(other.m_id, {}))
    {}

    ShaderProgram(const ShaderProgram &) = delete;

    ShaderProgram & operator=(const ShaderProgram &) = delete;

    ShaderProgram & operator=(ShaderProgram && other) noexcept
    {
        std::swap(index, other.index);
        std::swap(m_uniformCache, other.m_uniformCache);
        std::swap(m_vertexShaderIdx, other.m_vertexShaderIdx);
        std::swap(m_fragmentShaderIdx, other.m_fragmentShaderIdx);
        std::swap(m_id, other.m_id);
        return *this;
    }

    ~ShaderProgram()
    {
        glDeleteProgram(m_id);
    }

    [[nodiscard]] auto vertexShaderIdx() const -> SlotSetIndex { return m_vertexShaderIdx; }
    [[nodiscard]] auto fragmentShaderIdx() const -> SlotSetIndex { return m_fragmentShaderIdx; }
    [[nodiscard]] auto id() const -> GLuint { return m_id; }

    [[nodiscard]] auto link(const Shader & vertexShader,
                            const Shader & fragmentShader) -> std::expected<void, std::string>
    {
        assert(vertexShader.index == m_vertexShaderIdx && "Unexpected vertex shader");
        assert(fragmentShader.index == m_fragmentShaderIdx && "Unexpected fragment shader");

        int success;

        glLinkProgram(m_id);
        glGetProgramiv(m_id, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[1024];
            GLsizei infoLength;
            glGetProgramInfoLog(m_id, 1024, &infoLength, infoLog);
            return std::unexpected(std::string(infoLog, infoLength));
        }

        for (auto & cache: m_uniformCache | std::views::values)
        {
            cache.invalidateLocation();
        }

        return {};
    }

    auto setBool(const std::string_view & name, const GLboolean value) -> void
    {
        getOrCreateUniformCache(name).set(m_id, value);
    }

    auto setInt(const std::string_view & name, const GLint value) -> void
    {
        getOrCreateUniformCache(name).set(m_id, value);
    }

    auto setUint(const std::string_view & name, const GLuint value) -> void
    {
        getOrCreateUniformCache(name).set(m_id, value);
    }

    auto setFloat(const std::string_view & name, const GLfloat value) -> void
    {
        getOrCreateUniformCache(name).set(m_id, value);
    }

    auto setVec3(const std::string_view & name, const glm::vec3 value) -> void
    {
        getOrCreateUniformCache(name).set(m_id, value);
    }

    auto setVec4(const std::string_view & name, const glm::vec4 value) -> void
    {
        getOrCreateUniformCache(name).set(m_id, value);
    }

    auto setMat4(const std::string_view & name, const glm::mat4 & value) -> void
    {
        getOrCreateUniformCache(name).set(m_id, value);
    }

    auto setUniformBlock(const std::string_view & name, const GLuint uniformBlockBinding) -> void
    {
        getOrCreateUniformCache(name).set(m_id, UniformValue::BlockBinding(uniformBlockBinding));
    }

private:
    auto getOrCreateUniformCache(const std::string_view & name) -> UniformValue &
    {
#if __cpp_lib_associative_heterogeneous_insertion
        return m_uniformCache.try_emplace(name, name).first->second;
#else
        const auto it = m_uniformCache.find(name);
        if (it == m_uniformCache.end())
        {
            return m_uniformCache.try_emplace(std::string(name), name).first->second;
        }
        return it->second;
#endif
    }
};
