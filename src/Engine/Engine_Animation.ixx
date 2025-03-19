//
// Created by Simon Cros on 1/23/25.
//

module;

#include "tiny_gltf.h"

export module Engine:Animation;
import std;
import :AnimationSampler;

export class Animation
{
private:
    float m_duration;
    size_t m_samplerCount;
    std::vector<AnimationSampler> m_samplers;

    static auto initInputBuffer(const tinygltf::Model& model, int accessorIndex) -> AnimationSampler::InputBuffer;
    static auto initOutputBuffer(const tinygltf::Model& model, int accessorIndex) -> AnimationSampler::OutputBuffer;

public:
    Animation(const float duration,
              const size_t samplerCount,
              std::vector<AnimationSampler>&& samplers)
        : m_duration(duration),
          m_samplerCount(samplerCount),
          m_samplers(samplers)
    {
    }

    static auto Create(const tinygltf::Model& model, const tinygltf::Animation& animation) -> Animation;

    [[nodiscard]] auto duration() const -> float { return m_duration; }

    [[nodiscard]] auto sampler(const size_t index) const -> const AnimationSampler& { return m_samplers[index]; }
    [[nodiscard]] auto samplersCount() const -> size_t { return m_samplerCount; }
};
