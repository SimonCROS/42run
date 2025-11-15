//
// Created by Simon Cros on 31/01/2025.
//

export module Engine.FrameInfo;
import std.compat;
import Time;

export struct FrameInfo
{
    uint64_t frameCount;
    DurationType time;
    DurationType deltaTime;
};
