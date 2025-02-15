#include <string>
#include "glad/gl.h"
#include "Shader.h"
#include "../Utility/Expected.h"

auto Shader::Create(const GLenum type, const std::string_view& code) -> Expected<Shader, std::string>
{
    const GLuint id = glCreateShader(type);
    if (id == 0)
        return Unexpected("Failed to create new shader id");

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
        return Unexpected(std::string(infoLog, infoLength));
    }

    return Expected<Shader, std::string>{std::in_place, id};
}
