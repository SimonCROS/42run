#include <fstream>
#include <sstream>
#include <iostream>

#include "glad/gl.h"
#include "logger.hpp"
#include "Shader.hpp"

namespace Shader
{
    static bool TryCreateShader(const GLenum type, const char *code, GLuint *id)
    {
        GLuint shaderId = glCreateShader(type);

        if (shaderId == 0)
        {
            return false;
        }

        glShaderSource(shaderId, 1, &code, nullptr);
        *id = shaderId;
        return true;
    }

    static bool CompileShader(const GLuint id)
    {
        int success;
        char infoLog[1024];

        glCompileShader(id);
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            logError2(FTRUN_SHADER, FTRUN_COMPILATION_FAILED, infoLog);
            return false;
        }

        return true;
    }

    GLuint CreateShader(const std::string_view &code, const GLenum type)
    {
        GLuint id;
        if (!TryCreateShader(type, code.data(), &id))
        {
            return 0;
        }

        if (!CompileShader(id))
        {
            glDeleteShader(id);
            return 0;
        }

        return id;
    }

    void DestroyShader(GLuint id)
    {
        glDeleteShader(id);
    }

    bool TryGetShaderCode(const std::string_view &path, std::string *code)
    {
        std::ifstream file;
        std::stringstream stream;

        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            file.open(path.data());
            stream << file.rdbuf();
            file.close();

            *code = stream.str();
            return true;
        }
        catch (std::ifstream::failure &e)
        {
            logError2(FTRUN_SHADER, FTRUN_FILE_NOT_SUCCESSFULLY_READ, "(" << path << ") " << e.what());
            return false;
        }
    }
}
