#ifndef SHADER_H
#define SHADER_H

#include <string>

#include "glad/gl.h"

class Shader
{
public:
    GLuint id;

    Shader(const char *path, const GLenum type);
    Shader(const Shader&) = delete;
    ~Shader();

private:
    static bool try_create_shader(const GLenum type, const char *code, GLuint *id);
    static bool try_get_shader_code(const char *path, std::string *code);
    static bool compile_shader(const GLuint id);
};

#endif
