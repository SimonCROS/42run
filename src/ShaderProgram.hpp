#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <string>
#include <map>

#include <glm/mat4x4.hpp>

#include "glad/gl.h"
#include "Shader.hpp"

class ShaderProgram
{
public:
    GLuint id;

    ShaderProgram(const Shader &vertexShader, const Shader &fragmentShader);
    ShaderProgram(const ShaderProgram &) = delete;
    ~ShaderProgram();

    void Use() const;

    bool HasAttribute(const std::string_view& attribute) const;
    GLint GetAttributeLocation(const std::string_view& attribute) const;

    void SetBool(const std::string_view& name, const bool value) const;
    void SetInt(const std::string_view& name, const int value) const;
    void SetFloat(const std::string_view& name, const float value) const;
    void SetMat4(const std::string_view& name, const glm::mat4 &value) const;

private:
    std::map<std::string, GLint, std::less<>> attributes; // transparent std::less allow comparing key with std::string_view

    static bool LinkProgram(const GLuint id);
};

#endif
