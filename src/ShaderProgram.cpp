#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad/gl.h"
#include "logger.hpp"
#include "ShaderProgram.hpp"

GLuint ShaderProgram::currentShaderProgramId = 0;

ShaderProgram::ShaderProgram(const std::string_view& vertexCode, const std::string_view& fragmentCode) : id(0), _vertId(0), _fragId(0)
// id 0 is ignored with glDeleteProgram
{
    _vertId = Shader::CreateShader(vertexCode, GL_VERTEX_SHADER);
    _fragId = Shader::CreateShader(fragmentCode, GL_FRAGMENT_SHADER);

    if (_vertId == 0 || _fragId == 0)
    {
        // If one is not 0, delete it
        Shader::DestroyShader(_fragId);
        Shader::DestroyShader(_vertId);
        _fragId = 0;
        _vertId = 0;
        return;
    }

    id = glCreateProgram();
    glAttachShader(id, _vertId);
    glAttachShader(id, _fragId);

    if (!LinkProgram(id))
    {
        Destroy();
        return;
    }

    attributeLocations["POSITION"] = 0;
    attributeLocations["NORMAL"] = 1;
    attributeLocations["TANGENT"] = 2;
    attributeLocations["TEXCOORD_0"] = 3;
}

void ShaderProgram::Destroy()
{
    glDeleteProgram(id);
    Shader::DestroyShader(_fragId);
    Shader::DestroyShader(_vertId);
    id = 0;
    _fragId = 0;
    _vertId = 0;
}

void ShaderProgram::Use() const
{
    if (currentShaderProgramId != id)
    {
        glUseProgram(id);
        currentShaderProgramId = id;
    }
}

bool ShaderProgram::HasAttribute(const std::string& attribute) const
{
    return attributeLocations.find(attribute) != attributeLocations.cend();
}

GLint ShaderProgram::GetAttributeLocation(const std::string& attribute) const
{
    auto it = attributeLocations.find(attribute);
    return (it != attributeLocations.cend()) ? it->second : -1;
}

void ShaderProgram::EnableAttribute(GLint attribute) {
    _currentEnabledAttributes[attribute] = true;
}

void ShaderProgram::DisableAttribute(GLint attribute)
{
    _currentEnabledAttributes[attribute] = false;
}

void ShaderProgram::ApplyAttributeChanges() {
    for (size_t i = 0; i < 16; i++)
    {
        if (_currentEnabledAttributes[i]) // Should enable attribute
        {
            if (!_enabledAttributes[i]) // Attribute not already enabled
            {
                glEnableVertexAttribArray(i);
                _enabledAttributes[i] = true;
            }
            _currentEnabledAttributes[i] = false; // Reset for next call
        }
        else if (_enabledAttributes[i]) // Attribute was enabled
        {
            glDisableVertexAttribArray(i);
            _enabledAttributes[i] = false;
        }
    }
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
