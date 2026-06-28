//
// Created by Simon Cros on 4/29/25.
//

module;
#include "glad/gl.h"

export module OpenGL.Cubemap:Builder;
import std.compat;
import glToString;
import OpenGL.StateCache;
import Image;

export namespace OpenGL
{
    class Cubemap;

    struct CubemapCreateInfo
    {
        GLint internalFormat{GL_INVALID_ENUM};
        GLsizei size{0};
        GLint minFilter{GL_LINEAR};
        GLint magFilter{GL_LINEAR};
        GLint baseLevel{0};
        GLint maxLevel{0};
        const char * debugLabel{nullptr};
    };

    [[nodiscard]]
    auto createCubemap(StateCache & stateCache, const CubemapCreateInfo & info) -> std::expected<Cubemap, std::string>;
}
