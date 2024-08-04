#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <string>
#include <unordered_map>

#include <glm/mat4x4.hpp>

#include "glad/gl.h"
#include "Shader.hpp"
#include "utils/string_unordered_map.hpp"

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

    void SetBool(const std::string_view& name, const bool value);
    void SetInt(const std::string_view& name, const int value);
    void SetFloat(const std::string_view& name, const float value);
    void SetVec3(const std::string_view& name, const glm::vec3& value);
    void SetVec4(const std::string_view& name, const glm::vec4& value);
    void SetMat4(const std::string_view& name, const glm::mat4& value);
private:
    GLuint _vertId, _fragId;
    std::unordered_map<std::string, GLint> attributeLocations;
    string_unordered_map<bool> _bools;
    string_unordered_map<int> _ints;
    string_unordered_map<float> _floats;
    string_unordered_map<glm::vec3> _vec3s;
    string_unordered_map<glm::vec4> _vec4s;
    string_unordered_map<glm::mat4> _mat4s;
    bool _enabledAttributes[CUSTOM_MAX_VERTEX_ATTRIBUTES] = { 0 }; // Real map of enabled attributes
    bool _currentEnabledAttributes[CUSTOM_MAX_VERTEX_ATTRIBUTES] = { 0 }; // Attributes that will be enabled for the next reset

    static bool LinkProgram(const GLuint id);

    template<typename T>
    static bool StoreUniformValue(const std::string_view& name, T value, string_unordered_map<T>& map)
    {
        auto it =map.find(name.data()); // Should work without data on cpp20
        if (it != map.end() && it->second == value)
        {
            return false; // Value already set
        }

        map[std::string(name)] = value;
        return true;
    }
};

#endif
