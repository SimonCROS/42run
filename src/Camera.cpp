//
// Created by Simon Cros on 03/08/2024.
//

#include "Camera.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"
#include "glm/ext/matrix_transform.hpp"

Camera::Camera(const float speed, const glm::vec3& position, const glm::vec3& direction): Speed(speed),
    Position(position),
    Direction(direction)
{
    assert(glm::length2(Direction) != 0);
}

glm::mat4 Camera::GetView() const
{
    return glm::lookAt(Position, Position + Direction, Up);
}

glm::vec3 Camera::GetCrossDirection() const
{
    return glm::normalize(glm::cross(Direction, Up));
}
