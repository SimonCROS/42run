//
// Created by Simon Cros on 4/26/25.
//

module;
#include "stb_image.h"
#include "glad/gl.h"
#include <cerrno>

export module Image;
import std;
import glToString;

export class Image
{
private:
    const void* m_data;
    int m_width;
    int m_height;
    int m_nrChannels;
    bool m_isHdr;

public:
    Image() = delete;

    explicit Image(std::nullptr_t): m_data(nullptr), m_width(0), m_height(0), m_nrChannels(0), m_isHdr(false)
    {
    }

    Image(const int width, const int height, const int nrChannels, const void* data, const bool isHdr)
        : m_data(data), m_width(width), m_height(height), m_nrChannels(nrChannels), m_isHdr(isHdr)
    {
    }

    Image(const Image&) = delete;
    auto operator=(const Image&) -> Image& = delete;

    Image(Image&& other) noexcept
        : m_data(std::exchange(other.m_data, nullptr))
          , m_width(other.m_width)
          , m_height(other.m_height)
          , m_nrChannels(other.m_nrChannels)
          , m_isHdr(other.m_isHdr)
    {
    }

    auto operator=(Image&& other) noexcept -> Image&
    {
        if (this != &other)
        {
            std::swap(m_data, other.m_data);
            m_width = other.m_width;
            m_height = other.m_height;
            m_nrChannels = other.m_nrChannels;
            m_isHdr = other.m_isHdr;
        }
        return *this;
    }

    ~Image()
    {
        if (m_data != nullptr)
        {
            stbi_image_free(const_cast<void*>(m_data));
            m_data = nullptr;
        }
    }

    [[nodiscard]] static auto Create(const char* path) -> std::expected<Image, std::string>
    {
        int width;
        int height;
        int nrChannels;
        void* data;
        bool isHdr;

        const auto f = std::fopen(path, "rb");
        if (f == nullptr)
        {
            return std::unexpected(std::format("Failed to fopen at `{}`: {}", path, std::strerror(errno)));
        }

        isHdr = stbi_is_hdr_from_file(f);
        if (isHdr)
            data = stbi_loadf_from_file(f, &width, &height, &nrChannels, 0);
        else
            data = stbi_load_from_file(f, &width, &height, &nrChannels, 0);

        std::fclose(f);

        if (data == nullptr)
        {
            return std::unexpected(std::format("Failed to read image `{}`: {}", path, stbi_failure_reason()));
        }

        if (nrChannels < 1 || nrChannels > 4)
        {
            stbi_image_free(data);
            return std::unexpected(std::format("Unsupported image channels format (found {} channels)", nrChannels));
        }

        return std::expected<Image, std::string>(std::in_place, width, height, nrChannels, data, isHdr);
    }

    [[nodiscard]] constexpr auto width() const noexcept -> int { return m_width; }
    [[nodiscard]] constexpr auto height() const noexcept -> int { return m_height; }
    [[nodiscard]] constexpr auto nrChannels() const noexcept -> int { return m_nrChannels; }
    [[nodiscard]] constexpr auto data() const noexcept -> const void* { return m_data; }
    [[nodiscard]] constexpr auto isHdr() const noexcept -> bool { return m_isHdr; }

    [[nodiscard]] constexpr auto glFormat() const noexcept -> GLenum
    {
        switch (m_nrChannels)
        {
        case 1:
            return GL_RED;
        case 2:
            return GL_RG;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
        default:
            std::unreachable();
        }
    }

    [[nodiscard]] constexpr auto glType() const noexcept -> GLenum { return m_isHdr ? GL_FLOAT : GL_UNSIGNED_BYTE; }
};

export template <>
struct std::formatter<Image>
{
    template <class ParseContext>
    constexpr auto parse(ParseContext& ctx) -> typename ParseContext::iterator
    {
        return ctx.begin();
    }

    template <class FormatContext>
    auto format(const Image& obj, FormatContext& ctx) const -> typename FormatContext::iterator
    {
        return std::format_to(ctx.out(),
                              "Image{{width:{},height:{},glFormat:{},glType:{}}}",
                              obj.width(),
                              obj.height(),
                              glFormatToString(obj.glFormat()),
                              glTypeToString(obj.glType()));
    }
};
