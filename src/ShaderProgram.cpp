#include <fstream>
#include <sstream>
#include <iostream>

#include "glad/gl.h"
#include "logger.hpp"
#include "ShaderProgram.hpp"

ShaderProgram::ShaderProgram(const Shader &vertexShader, const Shader &fragmentShader) : id(0) // id 0 is ignored with glDeleteProgram
{
    if (vertexShader.id == 0 || fragmentShader.id == 0)
    {
        return;
    }

    id = glCreateProgram();
    glAttachShader(id, vertexShader.id);
    glAttachShader(id, fragmentShader.id);

    if (!link_program(id))
    {
        id = 0;
        glDeleteProgram(id);
        return;
    }
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}

void ShaderProgram::use() const
{
    glUseProgram(id);
}

void ShaderProgram::set_bool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void ShaderProgram::set_int(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void ShaderProgram::set_float(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

bool ShaderProgram::link_program(const GLuint id)
{
    int success;
    char infoLog[1024];

    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 1024, NULL, infoLog);
        logError2(FTRUN_SHADER_PROGRAM, FTRUN_COMPILATION_FAILED, infoLog);
        return false;
    }

    return true;
}
