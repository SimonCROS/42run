#ifndef SHADER_H
#define SHADER_H

#include <string>

#include "glad/gl.h"

namespace Shader
{
    GLuint CreateShader(const std::string_view& code, const GLenum type);
    void DestroyShader(GLuint shader);
    bool TryGetShaderCode(const std::string_view& path, std::string *code);
};

#endif
