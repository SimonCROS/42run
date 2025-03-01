//
// Created by Simon Cros on 3/1/25.
//

module;

#include <string>
#include <string_view>
#include <utility>
#include <expected>

#include "glad/gl.h"

export module OpenGL:Shader;

export class Shader
{
private:
    GLuint m_id;

public:
    static auto Create(GLenum type, const std::string_view& code) -> std::expected<Shader, std::string>
    {
        const GLuint id = glCreateShader(type);
        if (id == 0)
            return std::unexpected("Failed to create new shader id");

        const auto str = code.data();
        const auto length = static_cast<GLint>(code.size());
        glShaderSource(id, 1, &str, &length);
        glCompileShader(id);

        int success;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[1024];
            GLsizei infoLength;
            glGetShaderInfoLog(id, 1024, &infoLength, infoLog);
            glDeleteShader(id);
            return std::unexpected(std::string(infoLog, infoLength));
        }

        return std::expected<Shader, std::string>{std::in_place, id};
    }

    Shader(): m_id(0)
    {
    }

    explicit Shader(const GLuint id) : m_id(id)
    {
    }

    Shader(const Shader&) = delete;

    Shader(Shader&& other) noexcept : m_id(std::exchange(other.m_id, 0))
    {
    }

    ~Shader()
    {
        glDeleteShader(m_id);
    }

    auto operator=(const Shader&) -> Shader& = delete;

    auto operator=(Shader&& other) noexcept -> Shader&
    {
        std::swap(m_id, other.m_id);
        return *this;
    }

    [[nodiscard]] auto id() const -> GLuint { return m_id; }
};
