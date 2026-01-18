//
// Created by Simon Cros on 21/03/2025.
//

module;
#include "glad/gl.h"
#include "stb_image.h"

export module OpenGL:Texture2D;
import std;

export class Texture2D
{
private:
    GLuint m_textureId;

    explicit Texture2D(const GLuint id) : m_textureId(id)
    {
    }

public:
    Texture2D(const Texture2D&) = delete;
    Texture2D(Texture2D&& other) noexcept;
    ~Texture2D();

    auto operator=(const Texture2D&) -> Texture2D& = delete;

    static auto Create8u(const std::string& path) -> Texture2D;

    static auto Create32f(const std::string& path) -> Texture2D;

    [[nodiscard]] auto textureId() const -> GLuint
    {
        return m_textureId;
    }

    auto bind() const -> void
    {
        glBindTexture(GL_TEXTURE_2D, m_textureId);
    }
};
