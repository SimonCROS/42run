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

    class Texture2DBuilder
    {
    private:
        StateCache * m_stateCache;
        GLint m_internalFormat;
        GLsizei m_width;
        GLsizei m_height;
        GLint m_wrapS = GL_CLAMP_TO_EDGE;
        GLint m_wrapT = GL_CLAMP_TO_EDGE;
        GLint m_minFilter = GL_LINEAR;
        GLint m_magFilter = GL_LINEAR;
        const char * m_debugLabel;

    public:
        explicit Texture2DBuilder(StateCache * stateCache) noexcept
            : m_stateCache(stateCache),
              m_internalFormat(0),
              m_width(0),
              m_height(0),
              m_debugLabel(nullptr)
        {}

        // [[nodiscard]]
        // auto fromImage(const Image & image, const GLint internalFormat) noexcept -> Texture2DBuilder &
        // {
        //     m_internalFormat = internalFormat;
        //     m_width = image.width();
        //     m_height = image.height();
        //     m_data = image.data();
        //
        //     if (!isBaseInternalFormat(internalFormat))
        //     {
        //         if (image.isHdr())
        //         {
        //             if (!isFloatInternalFormat(internalFormat))
        //                 std::println(
        //                     "[Warning] HDR image data (float) is being uploaded to an integer internal format ({}). This will cause loss of precision and dynamic range.",
        //                     glFormatToString(internalFormat));
        //         }
        //         else
        //         {
        //             if (!isIntegerInternalFormat(internalFormat))
        //                 std::println(
        //                     "[Warning] LDR image data (byte) is being uploaded to a float internal format ({}). This wastes memory and bandwidth without increasing quality.",
        //                     glFormatToString(internalFormat));
        //         }
        //     }
        //
        //     return *this;
        // }

        [[nodiscard]]
        auto size(const GLsizei width, const GLsizei height) noexcept -> Texture2DBuilder &
        {
            m_width = width;
            m_height = height;
            return *this;
        }

        [[nodiscard]]
        auto internalFormat(const GLint internalFormat) noexcept -> Texture2DBuilder &
        {
            m_internalFormat = internalFormat;
            return *this;
        }

        [[nodiscard]]
        auto wrapping(const GLint wrapS, const GLint wrapT) noexcept -> Texture2DBuilder &
        {
            m_wrapS = wrapS;
            m_wrapT = wrapT;
            return *this;
        }

        [[nodiscard]]
        auto filtering(const GLint minFilter, const GLint magFilter) noexcept -> Texture2DBuilder &
        {
            m_minFilter = minFilter;
            m_magFilter = magFilter;
            return *this;
        }

        [[nodiscard]]
        auto debugLabel(const char *label) noexcept -> Texture2DBuilder &
        {
            m_debugLabel = label;
            return *this;
        }

        [[nodiscard]]
        auto build() const -> std::expected<Texture2D, std::string>;
    };
}
