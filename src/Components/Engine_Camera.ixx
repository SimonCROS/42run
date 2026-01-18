//
// Created by loumarti on 1/15/25.
//

module;

#include "glm/glm.hpp"

export module Engine:Camera;
import :Component;

export enum ViewMode
{
    COLOR,
    TEXTURE
};

export class Camera final : public Component
{
private:
    ViewMode m_mode;
    glm::mat4 m_projectionMatrix{};

public:
    explicit Camera(Object& object, uint32_t width, uint32_t height, float fov);

    [[nodiscard]] auto getViewMode() const -> ViewMode { return m_mode; }
    [[nodiscard]] auto projectionMatrix() const -> const glm::mat4& { return m_projectionMatrix; }

    [[nodiscard]] glm::mat4 computeViewMatrix() const;
};
