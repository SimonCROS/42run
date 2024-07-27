#include <fstream>
#include <sstream>
#include <iostream>

#include "glad/gl.h"
#include "logger.hpp"
#include "Shader.hpp"

Shader::Shader(const char *path, const GLenum type) : id(0) // id 0 is ignored with glDeleteShader
{
    std::string code;
    if (!try_get_shader_code(path, &code))
    {
        return;
    }

    if (!try_create_shader(type, code.c_str(), &id))
    {
        id = 0;
        return;
    }

    if (!compile_shader(id))
    {
        glDeleteShader(id);
        id = 0;
        return;
    }
}

Shader::~Shader()
{
    glDeleteShader(id);
}

bool Shader::try_create_shader(const GLenum type, const char *code, GLuint *id)
{
    GLuint shaderId = glCreateShader(type);

    if (shaderId == 0)
    {
        return false;
    }

    glShaderSource(shaderId, 1, &code, NULL);
    *id = shaderId;
    return true;
}

bool Shader::try_get_shader_code(const char *path, std::string *code)
{
    std::ifstream file;
    std::stringstream stream;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        file.open(path);
        stream << file.rdbuf();
        file.close();

        *code = stream.str();
        return true;
    }
    catch (std::ifstream::failure &e)
    {
        logError2(FTRUN_SHADER, FTRUN_FILE_NOT_SUCCESSFULLY_READ, "(" << path << ") " << e.what());
        return false;
    }
}

bool Shader::compile_shader(const GLuint id)
{
    int success;
    char infoLog[1024];

    glCompileShader(id);
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 1024, NULL, infoLog);
        logError2(FTRUN_SHADER, FTRUN_COMPILATION_FAILED, infoLog);
        return false;
    }

    return true;
}
