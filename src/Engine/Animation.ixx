//
// Created by Simon Cros on 1/23/25.
//

module;

#include "tiny_gltf.h"

export module Engine.Animation;
import std;
import Engine.AnimationSampler;
import Engine.AnimationChannel;

export class Animation
{
private:
    std::string m_name;
    float m_duration;
    std::vector<AnimationChannel> m_channels;
    std::vector<AnimationSampler> m_samplers;

    static auto initInputBuffer(const tinygltf::Model &model, int accessorIndex) -> AnimationSampler::InputBuffer;

    static auto initOutputBuffer(const tinygltf::Model &model, int accessorIndex) -> AnimationSampler::OutputBuffer;

public:
    Animation(const float duration,
              std::vector<AnimationChannel> &&channels,
              std::vector<AnimationSampler> &&samplers)
        : m_duration(duration),
          m_channels(std::move(channels)),
          m_samplers(std::move(samplers)) {}

    static auto Create(const tinygltf::Model &model, const tinygltf::Animation &animation) -> Animation;

    [[nodiscard]] auto name() const -> const std::string & { return m_name; }
    [[nodiscard]] auto duration() const -> float { return m_duration; }

    [[nodiscard]] auto channel(const size_t index) const -> const AnimationChannel & { return m_channels[index]; }
    [[nodiscard]] auto channelsCount() const -> size_t { return m_channels.size(); }
    [[nodiscard]] auto sampler(const size_t index) const -> const AnimationSampler & { return m_samplers[index]; }
    [[nodiscard]] auto samplersCount() const -> size_t { return m_samplers.size(); }
};
