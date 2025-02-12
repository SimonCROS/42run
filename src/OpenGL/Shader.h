#ifndef SHADER_H
#define SHADER_H

#include <string_view>
#include <utility>

#include "glad/gl.h"
#include "Expected.h"

class Shader
{
private:
    GLuint m_id;

public:
    static auto Create(GLenum type, const std::string_view& code) -> Expected<Shader, std::string>;

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

#endif
