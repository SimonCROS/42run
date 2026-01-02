//
// Created by Simon Cros on 4/29/25.
//

module;
#include "glad/gl.h"

export module OpenGL.Cubemap2:Builder;
import std.compat;
import glToString;
import OpenGL.StateCache;
import Image;

export namespace OpenGL
{
    class Cubemap2;

    class CubemapBuilder
    {
    private:
        StateCache* m_stateCache;
        GLint m_internalFormat;
        GLsizei m_size;
        std::array<const void*, 6> m_faceData;
        GLenum m_format;
        GLenum m_type;
        GLint m_minFilter = GL_LINEAR_MIPMAP_LINEAR;
        GLint m_magFilter = GL_LINEAR;

    public:
        explicit CubemapBuilder(StateCache* stateCache) noexcept
            : m_stateCache(stateCache)
              , m_internalFormat(0)
              , m_size(0)
              , m_faceData{nullptr}
              , m_format(0)
              , m_type(0)
        {
        }

        [[nodiscard]]
        auto fromImages(const std::array<Image*, 6>& images, const GLint internalFormat) -> CubemapBuilder&
        {
            m_internalFormat = internalFormat;
            m_size = images[0]->width();
            m_format = images[0]->glFormat();
            m_type = images[0]->glType();

            for (size_t i = 0; i < 6; ++i)
            {
                const auto* image = images[i];
                if (image->width() != image->height())
                    std::println(
                        "[Warning] Image {} is not square.",
                        i);
                if (image->width() != m_size || image->glFormat() != m_format || image->glType() != m_type)
                    std::println(
                        "[Warning] Image {} is not same size, format or type as image 0.",
                        i);

                m_faceData[i] = image->data();
            }

            return *this;
        }

        [[nodiscard]]
        auto withSize(const GLsizei size) noexcept -> CubemapBuilder&
        {
            m_size = size;
            return *this;
        }

        [[nodiscard]]
        auto withFormat(const GLint internalFormat, const GLenum format, const GLenum type) noexcept -> CubemapBuilder&
        {
            m_internalFormat = internalFormat;
            m_format = format;
            m_type = type;
            return *this;
        }

        [[nodiscard]]
        auto withFaceData(const std::array<const void*, 6>& faceData) noexcept -> CubemapBuilder&
        {
            m_faceData = faceData;
            return *this;
        }

        [[nodiscard]]
        auto withFiltering(const GLint minFilter, const GLint magFilter) noexcept -> CubemapBuilder&
        {
            m_minFilter = minFilter;
            m_magFilter = magFilter;
            return *this;
        }

        [[nodiscard]]
        auto build() const -> std::expected<Cubemap2, std::string>;
    };
}
