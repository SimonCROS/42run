#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

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

void ShaderProgram::set_bool(const std::string &name, const bool value) const { set_bool(name.c_str(), value); }
void ShaderProgram::set_bool(const char *name, const bool value) const
{
    glUniform1i(glGetUniformLocation(id, name), (int)value);
}

void ShaderProgram::set_int(const std::string &name, const int value) const { set_int(name.c_str(), value); }
void ShaderProgram::set_int(const char *name, const int value) const
{
    glUniform1i(glGetUniformLocation(id, name), value);
}

void ShaderProgram::set_float(const std::string &name, const float value) const { set_float(name.c_str(), value); }
void ShaderProgram::set_float(const char *name, const float value) const
{
    glUniform1f(glGetUniformLocation(id, name), value);
}

void ShaderProgram::set_mat4(const std::string &name, const glm::mat4 &value) const { set_mat4(name.c_str(), value); }
void ShaderProgram::set_mat4(const char *name, const glm::mat4 &value) const
{
    glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE, glm::value_ptr(value));
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
