//
// Created by Simon Cros on 31/01/2025.
//

export module Engine:FrameInfo;
import std.compat;

export using ClockType = std::chrono::steady_clock;
export using TimePoint = ClockType::time_point;
export using DurationType = std::chrono::duration<float>;

export struct FrameInfo
{
    uint64_t frameCount;
    DurationType time;
    DurationType deltaTime;
};
