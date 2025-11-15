//
// Created by Simon Cros on 1/23/25.
//

module;

#include "tiny_gltf.h"
#include "glad/gl.h"

module Engine.Animation;

auto Animation::initInputBuffer(const tinygltf::Model &model, const int accessorIndex)
    -> AnimationSampler::InputBuffer
{
    const auto &accessor = model.accessors[accessorIndex];
    assert(accessor.type == TINYGLTF_TYPE_SCALAR);
    assert(accessor.componentType == GL_FLOAT);
    const auto &bufferView = model.bufferViews[accessor.bufferView];
    const auto &buffer = model.buffers[bufferView.buffer];

    const size_t offset = bufferView.byteOffset + accessor.byteOffset;
    constexpr size_t attributeSize = sizeof(GLfloat); // accessor.componentType MUST be GL_FLOAT
    const size_t byteStride = accessor.ByteStride(bufferView);
    assert(byteStride % sizeof(GLfloat) == 0 && "byteStride must be a multiple of attributeSize");

    const size_t length = byteStride * accessor.count;
    assert(length > 0);
    assert(accessor.byteOffset + length <= bufferView.byteLength);

    const auto data = reinterpret_cast<const GLfloat *>(buffer.data.data() + offset);
    return {
        .size = accessor.count,
        .attributeStride = byteStride / sizeof(GLfloat),
        .data = data,
    };
}

auto Animation::initOutputBuffer(const tinygltf::Model &model, const int accessorIndex)
    -> AnimationSampler::OutputBuffer
{
    const auto &accessor = model.accessors[accessorIndex];
    const auto &bufferView = model.bufferViews[accessor.bufferView];
    const auto &buffer = model.buffers[bufferView.buffer];

    const size_t offset = bufferView.byteOffset + accessor.byteOffset;
    const size_t attributeSize =
            tinygltf::GetNumComponentsInType(accessor.type) * tinygltf::GetComponentSizeInBytes(accessor.componentType);
    const size_t byteStride = accessor.ByteStride(bufferView);

    const size_t length = byteStride * accessor.count;
    assert(length > 0);
    assert(accessor.byteOffset + length <= bufferView.byteLength);

    const GLubyte *bytes = buffer.data.data() + offset;
    return {
        .size = length,
        .byteStride = byteStride,
        .data = bytes,
    };
}

auto Animation::Create(const tinygltf::Model &model, const tinygltf::Animation &animation) -> Animation
{
    float duration = 0;
    std::vector<AnimationChannel> channels;
    std::vector<AnimationSampler> samplers;

    channels.reserve(animation.channels.size());
    for (const auto &i: animation.channels)
    {
        AnimationChannelType type;
        if (i.target_path == "translation")
        {
            type = AnimationChannelType::Translation;
        } else if (i.target_path == "rotation")
        {
            type = AnimationChannelType::Rotation;
        } else if (i.target_path == "scale")
        {
            type = AnimationChannelType::Scale;
        } else
        {
            std::println(stderr, "Unsupported target_path: {}", i.target_path);
            continue;
        }
        channels.emplace_back(i.sampler, i.target_node, type);
    }

    samplers.reserve(animation.samplers.size());
    for (const auto &i: animation.samplers)
    {
        auto input = initInputBuffer(model, i.input);
        auto output = initOutputBuffer(model, i.output);

        const auto &inserted = samplers.emplace_back(input, output);
        duration = std::max(duration, inserted.duration());
    }

    return {
        duration,
        std::move(channels),
        std::move(samplers),
    };
}
