#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <string>

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

    void use() const;

    void set_bool(const char *name, const bool value) const;
    void set_bool(const std::string &name, const bool value) const;
    void set_int(const char *name, const int value) const;
    void set_int(const std::string &name, const int value) const;
    void set_float(const char *name, const float value) const;
    void set_float(const std::string &name, const float value) const;
    void set_mat4(const char *name, const glm::mat4 &value) const;
    void set_mat4(const std::string &name, const glm::mat4 &value) const;

private:
    static bool link_program(const GLuint id);
};

#endif
