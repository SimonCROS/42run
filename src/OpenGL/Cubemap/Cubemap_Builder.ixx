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

    class CubemapBuilder
    {
    private:
        StateCache * m_stateCache;
        GLint m_internalFormat;
        GLsizei m_size;
        GLuint m_baseLevel;
        GLuint m_maxLevel;
        GLint m_minFilter = GL_LINEAR;
        GLint m_magFilter = GL_LINEAR;
        const char * m_debugLabel;

    public:
        explicit CubemapBuilder(StateCache * stateCache) noexcept
            : m_stateCache(stateCache),
              m_internalFormat(0),
              m_size(0),
              m_baseLevel(0),
              m_maxLevel(0),
              m_debugLabel(nullptr)
        {}

        // [[nodiscard]]
        // auto fromImages(const std::array<Image*, 6>& images, const GLint internalFormat) -> CubemapBuilder&
        // {
        //     m_internalFormat = internalFormat;
        //     m_size = images[0]->width();
        //     m_format = images[0]->glFormat();
        //     m_type = images[0]->glType();
        //
        //     for (size_t i = 0; i < 6; ++i)
        //     {
        //         const auto* image = images[i];
        //         if (image->width() != image->height())
        //             std::println(
        //                 "[Warning] Image {} is not square.",
        //                 i);
        //         if (image->width() != m_size || image->glFormat() != m_format || image->glType() != m_type)
        //             std::println(
        //                 "[Warning] Image {} is not same size, format or type as image 0.",
        //                 i);
        //
        //         m_faceData[i] = image->data();
        //     }
        //
        //     return *this;
        // }

        [[nodiscard]]
        auto size(const GLsizei size) noexcept -> CubemapBuilder &
        {
            m_size = size;
            return *this;
        }

        [[nodiscard]]
        auto internalFormat(const GLint internalFormat) noexcept -> CubemapBuilder &
        {
            m_internalFormat = internalFormat;
            return *this;
        }

        [[nodiscard]]
        auto filtering(const GLint minFilter, const GLint magFilter) noexcept -> CubemapBuilder &
        {
            m_minFilter = minFilter;
            m_magFilter = magFilter;
            return *this;
        }

        [[nodiscard]]
        auto baseLevel(const GLuint baseLevel) noexcept -> CubemapBuilder &
        {
            m_baseLevel = baseLevel;
            return *this;
        }

        [[nodiscard]]
        auto maxLevel(const GLuint maxLevel) noexcept -> CubemapBuilder &
        {
            m_maxLevel = maxLevel;
            return *this;
        }

        [[nodiscard]]
        auto debugLabel(const char * label) noexcept -> CubemapBuilder &
        {
            m_debugLabel = label;
            return *this;
        }

        [[nodiscard]]
        auto build() const -> std::expected<Cubemap, std::string>;
    };
}
