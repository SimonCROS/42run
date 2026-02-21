//
// Created by Simon Cros on 01/03/2025.
//

module;
#include "glad/gl.h"

export module OpenGL;

export import :Debug;
export import :IndicesBuffer;
export import :VertexArray;
export import :VertexBuffer;

export import Shader;
export import ShaderFile;
export import ShaderFlags;
export import ShaderManager;
export import ShaderProgram;

// TODO move
export namespace OpenGL
{
    constexpr auto formatComponentsCount(const GLenum format) -> uint32_t
    {
        switch (format)
        {
            case GL_RED:
                return 1;
            case GL_RG:
                return 2;
            case GL_RGB:
            case GL_BGR:
                return 3;
            case GL_RGBA:
            case GL_BGRA:
                return 4;
            default:
                assert(false && "Unknown format");
        }
    }

    constexpr auto typeSize(const GLenum type) -> uint32_t
    {
        switch (type)
        {
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
                return 1;
            case GL_UNSIGNED_SHORT:
            case GL_SHORT:
            case GL_HALF_FLOAT:
                return 2;
            case GL_UNSIGNED_INT:
            case GL_INT:
            case GL_FLOAT:
                return 4;
            case GL_DOUBLE:
                return 8;
            default:
                assert(false && "Unknown format");
        }
    }
}
