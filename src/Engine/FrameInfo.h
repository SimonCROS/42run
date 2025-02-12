//
// Created by Simon Cros on 31/01/2025.
//

#ifndef FRAMEINFO_H
#define FRAMEINFO_H

#include <chrono>

using ClockType = std::chrono::steady_clock;
using TimePoint = ClockType::time_point;
using DurationType = std::chrono::duration<float>;

struct FrameInfo
{
    uint64_t frameCount;
    DurationType time;
    DurationType deltaTime;
};

#endif //FRAMEINFO_H
