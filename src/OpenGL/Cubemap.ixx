//
// Created by Simon Cros on 21/03/2025.
//

module;
#include "glad/gl.h"
#include "stb_image.h"

export module OpenGL:Cubemap;
import std;

export class Cubemap
{
private:
    GLuint m_textureId;

    explicit Cubemap(const GLuint id) : m_textureId(id)
    {
    }

public:
    Cubemap(const Cubemap&) = delete;
    Cubemap(Cubemap&& other) noexcept;
    ~Cubemap();

    auto operator=(const Cubemap&) -> Cubemap& = delete;

    static auto Create(const std::vector<std::string>& faces) -> Cubemap;

    [[nodiscard]] auto textureId() const -> GLuint
    {
        return m_textureId;
    }

    auto bind() const -> void
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureId);
    }
};
