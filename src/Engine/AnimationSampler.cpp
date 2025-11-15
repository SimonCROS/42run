//
// Created by Simon Cros on 22/01/2025.
//

module Engine.AnimationSampler;
import std.compat;
import Utility;

using difference_type = StridedIterator<float*>::difference_type;

auto AnimationSampler::getInput(const float time) const -> InputResult
{
    const auto begin = StridedIterator(m_input.data, static_cast<difference_type>(m_input.attributeStride));
    const auto end = begin + static_cast<difference_type>(m_input.size);
    const auto upperBound = std::upper_bound(begin, end, time);

    // Clamp to first and last
    if (upperBound == begin)
        return {0, 0, 0};
    if (upperBound == end)
        return {m_input.size - 1, m_input.size - 1, 1};

    const auto next = upperBound;
    const auto prev = upperBound - 1;

    const auto prevVal = *prev;
    const auto nextVal = *next;
    const auto prevIndex = prev - begin;
    const auto nextIndex = next - begin;

    const float stepRatio = (nextVal != prevVal) ? ((time - prevVal) / (nextVal - prevVal)) : 0;
    return {static_cast<size_t>(prevIndex), static_cast<size_t>(nextIndex), stepRatio};
}

AnimationSampler::AnimationSampler(const InputBuffer& input, const OutputBuffer& output)
    : m_input(input), m_output(output)
{
    m_duration = input.data[(input.size - 1) * input.attributeStride];
}
