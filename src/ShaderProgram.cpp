#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/gl.h"
#include "logger.hpp"
#include "ShaderProgram.hpp"

ShaderProgram::ShaderProgram(const Shader& vertexShader, const Shader& fragmentShader) : id(0)
// id 0 is ignored with glDeleteProgram
{
    if (vertexShader.id == 0 || fragmentShader.id == 0)
    {
        return;
    }

    id = glCreateProgram();
    glAttachShader(id, vertexShader.id);
    glAttachShader(id, fragmentShader.id);

    if (!LinkProgram(id))
    {
        glDeleteProgram(id);
        id = 0;
        return;
    }

    attributes["POSITION"] = 0;
    attributes["NORMAL"] = 1;
    attributes["TEXCOORD_0"] = 2;
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(id);
}

void ShaderProgram::Use() const
{
    glUseProgram(id);
}

bool ShaderProgram::HasAttribute(const std::string_view& attribute) const
{
    return attributes.find(attribute) != attributes.cend();
}

GLint ShaderProgram::GetAttributeLocation(const std::string_view& attribute) const
{
    auto it = attributes.find(attribute);
    return (it != attributes.cend()) ? it->second : -1;
}

void ShaderProgram::SetBool(const std::string_view& name, const bool value) const
{
    glUniform1i(glGetUniformLocation(id, name.data()), (int)value);
}

void ShaderProgram::SetInt(const std::string_view& name, const int value) const
{
    glUniform1i(glGetUniformLocation(id, name.data()), value);
}

void ShaderProgram::SetFloat(const std::string_view& name, const float value) const
{
    glUniform1f(glGetUniformLocation(id, name.data()), value);
}

void ShaderProgram::SetVec3(const std::string_view& name, const glm::vec3& value) const
{
    glUniform3f(glGetUniformLocation(id, name.data()), value.x, value.y, value.z);
}

void ShaderProgram::SetVec4(const std::string_view& name, const glm::vec4& value) const
{
    glUniform4f(glGetUniformLocation(id, name.data()), value.x, value.y, value.z, value.w);
}

void ShaderProgram::SetMat4(const std::string_view& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(id, name.data()), 1, GL_FALSE, glm::value_ptr(value));
}

bool ShaderProgram::LinkProgram(const GLuint id)
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
