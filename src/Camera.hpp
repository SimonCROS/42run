//
// Created by Simon Cros on 03/08/2024.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "glm/glm.hpp"


class Camera
{
public:
    float Speed;
    glm::vec3 Position;
    glm::vec3 Direction;

    Camera() = delete;
    Camera(float speed, const glm::vec3& position, const glm::vec3& direction);

    [[nodiscard]] glm::mat4 GetView() const;
    [[nodiscard]] glm::vec3 GetCrossDirection() const;

private:
    const glm::vec3 Up = glm::vec3(0, 1, 0);
};


#endif //CAMERA_HPP
