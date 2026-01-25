//
// Created by Simon Cros on 22/01/2025.
//

module;

#include "glad/gl.h"

export module Engine.AnimationSampler;
import glm;

export class AnimationSampler
{
public:
    struct InputBuffer
    {
        size_t size;
        size_t attributeStride;
        const GLfloat *data;
    };

    struct OutputBuffer
    {
        size_t size;
        GLsizei byteStride;
        const GLubyte *data;
    };

private:
    GLfloat m_duration{};
    InputBuffer m_input;
    OutputBuffer m_output;

    struct InputResult
    {
        size_t prevIndex;
        size_t nextIndex;
        float t;
    };

    [[nodiscard]] auto getInput(float time) const -> InputResult;

    template<class T>
    [[nodiscard]] auto getOutputPtr(const size_t index) const -> const T *
    {
        return reinterpret_cast<const T *>(m_output.data + index * m_output.byteStride);
    }

public:
    AnimationSampler(const InputBuffer &input, const OutputBuffer &output);

    [[nodiscard]] auto duration() const -> float { return m_duration; }

    [[nodiscard]] auto vec3(const float time) const -> glm::vec3
    {
        const auto result = getInput(time);
        return glm::mix(*getOutputPtr<glm::vec3>(result.prevIndex),
                        *getOutputPtr<glm::vec3>(result.nextIndex),
                        result.t);
    }

    [[nodiscard]] auto vec4(const float time) const -> glm::vec4
    {
        const auto result = getInput(time);
        return glm::mix(*getOutputPtr<glm::vec4>(result.prevIndex),
                        *getOutputPtr<glm::vec4>(result.nextIndex),
                        result.t);
    }

    [[nodiscard]] auto quat(const float time) const -> glm::quat
    {
        const auto result = getInput(time);
        const auto prev = getOutputPtr<float>(result.prevIndex);
        const auto next = getOutputPtr<float>(result.nextIndex);
        return glm::slerp(glm::quat(prev[3], prev[0], prev[1], prev[2]),
                          glm::quat(next[3], next[0], next[1], next[2]),
                          result.t);
    }
};
