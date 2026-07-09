//
// Created by Simon Cros on 4/29/25.
//

module;
#include "glad/gl.h"
#include <cassert>

export module OpenGL.Texture2D:Builder;
import std;
import glToString;
import OpenGL.StateCache;
import Image;

constexpr auto isBaseInternalFormat(const GLint internalFormat) -> bool
{
    switch (internalFormat)
    {
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
        case GL_RED:
        case GL_RG:
        case GL_RGB:
        case GL_RGBA:
            return true;
        default:
            return false;
    }
}

constexpr auto isFloatInternalFormat(const GLint internalFormat) -> bool
{
    switch (internalFormat)
    {
        case GL_R16F:
        case GL_R32F:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_RGBA16F:
        case GL_RGBA32F:
            return true;
        default:
            return false;
    }
}

constexpr auto isIntegerInternalFormat(const GLint internalFormat) -> bool
{
    switch (internalFormat)
    {
        case GL_R8:
        case GL_R16:
        case GL_RG8:
        case GL_RG16:
        case GL_RGB8:
        case GL_RGB16:
        case GL_RGBA8:
        case GL_RGBA16:
            return true;
        default:
            return false;
    }
}

export namespace OpenGL
{
    class Texture2D;

    struct Texture2DCreateInfo
    {
        GLint internalFormat{GL_INVALID_ENUM};
        GLsizei m_width{0};
        GLsizei m_height{0};
        GLint m_wrapS{GL_CLAMP_TO_EDGE};
        GLint m_wrapT{GL_CLAMP_TO_EDGE};
        GLint minFilter{GL_LINEAR};
        GLint magFilter{GL_LINEAR};
        GLint baseLevel{0};
        GLint maxLevel{0};
        const char * debugLabel{nullptr};
    };

    [[nodiscard]]
    auto createTexture2D(StateCache & stateCache, const Texture2DCreateInfo & info) -> std::expected<Texture2D, std::string>;
}
