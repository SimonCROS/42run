//
// Created by Simon Cros on 1/17/25.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct Transform
{
    glm::vec3 translation{};
    glm::quat rotation = glm::identity<glm::quat>();
    glm::vec3 scale{1.0f};

    [[nodiscard]] auto trs() const -> glm::mat4
    {
        auto mat = glm::identity<glm::mat4>();
        mat = glm::translate(mat, translation);
        mat *= glm::mat4_cast(rotation);
        mat = glm::scale(mat, scale);
        return mat;
    }
};

#endif //TRANSFORM_H
