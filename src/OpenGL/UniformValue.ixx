//
// Created by scros on 12/7/25.
//

module;

#include "glad/gl.h"

export module UniformValue;
import std;
import glm;

export class UniformValue
{
private:
    static constexpr GLint Unset = -2;
    static constexpr GLint NotFound = -1;

    std::string m_name;
    std::variant<std::monostate, GLboolean, GLfloat, GLint, GLuint, glm::vec2, glm::vec3, glm::vec4, glm::mat4> m_value;
    GLint m_location;

public:
    explicit UniformValue(const std::string_view & m_name) : m_name(m_name), m_location(Unset) {}

    auto invalidateLocation() -> void
    {
        m_location = Unset;
        m_value = std::monostate{};
    }

    template<typename T>
    auto hasValue(const T & value) -> bool
    {
        return std::holds_alternative<T>(m_value) && std::get<T>(m_value) == value;
    }

    template<typename T>
    auto set(const GLint program, const T & value) -> void
    {
        if (m_location == NotFound || hasValue(value))
        {
            return;
        }

        m_value = value;

        if (m_location == Unset)
        {
            m_location = glGetUniformLocation(program, m_name.c_str());
        }

        if constexpr (std::is_same_v<T, GLboolean>)
        {
            glUniform1i(m_location, static_cast<GLint>(value));
        }
        else if constexpr (std::is_same_v<T, GLfloat>)
        {
            glUniform1f(m_location, value);
        }
        else if constexpr (std::is_same_v<T, GLint>)
        {
            glUniform1i(m_location, value);
        }
        else if constexpr (std::is_same_v<T, GLuint>)
        {
            glUniform1ui(m_location, value);
        }
        else if constexpr (std::is_same_v<T, glm::vec2>)
        {
            glUniform2f(m_location, value.x, value.y);
        }
        else if constexpr (std::is_same_v<T, glm::vec3>)
        {
            glUniform3f(m_location, value.x, value.y, value.z);
        }
        else if constexpr (std::is_same_v<T, glm::vec4>)
        {
            glUniform4f(m_location, value.x, value.y, value.z, value.w);
        }
        else if constexpr (std::is_same_v<T, glm::mat4>)
        {
            glUniformMatrix4fv(m_location, 1, GL_FALSE, glm::gtc::value_ptr(value));
        }
    }
};
