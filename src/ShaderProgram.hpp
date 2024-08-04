#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <string>
#include <unordered_map>

#include <glm/mat4x4.hpp>

#include "glad/gl.h"
#include "Shader.hpp"

#define CUSTOM_MAX_VERTEX_ATTRIBUTES 16

class ShaderProgram
{
public:
    static GLuint currentShaderProgramId;
    GLuint id;

    ShaderProgram(const std::string_view& vertexCode, const std::string_view& fragmentCode);

    void Destroy();

    void Use() const;

    bool HasAttribute(const std::string& attribute) const;
    GLint GetAttributeLocation(const std::string& attribute) const;
    void EnableAttribute(GLuint attribute);
    void DisableAttribute(GLuint attribute);
    void ApplyAttributeChanges();

    void SetBool(const std::string_view& name, const bool value) const;
    void SetInt(const std::string_view& name, const int value) const;
    void SetFloat(const std::string_view& name, const float value) const;
    void SetVec3(const std::string_view& name, const glm::vec3& value) const;
    void SetVec4(const std::string_view& name, const glm::vec4& value) const;
    void SetMat4(const std::string_view& name, const glm::mat4& value) const;
private:
    GLuint _vertId, _fragId;
    std::unordered_map<std::string, GLint> attributeLocations;
    bool _enabledAttributes[CUSTOM_MAX_VERTEX_ATTRIBUTES] = { 0 }; // Real map of enabled attributes
    bool _currentEnabledAttributes[CUSTOM_MAX_VERTEX_ATTRIBUTES] = { 0 }; // Attributes that will be enabled for the next reset

    static bool LinkProgram(const GLuint id);
};

#endif
