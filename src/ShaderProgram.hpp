#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include <string>

#include "glad/gl.h"
#include "Shader.hpp"

class ShaderProgram
{
public:
    GLuint id;

    ShaderProgram(const Shader& vertexShader, const Shader& fragmentShader);
    ShaderProgram(const ShaderProgram&) = delete;
    ~ShaderProgram();

    void use() const;

    void set_bool(const std::string &name, bool value) const;
    void set_int(const std::string &name, int value) const;
    void set_float(const std::string &name, float value) const;

private:
    static bool link_program(const GLuint id);
};

#endif
