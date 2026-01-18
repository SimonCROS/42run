//
// Created by Simon Cros on 31/01/2025.
//

export module Engine.FrameInfo;
import std.compat;
import Time;

export struct FrameInfo
{
    uint64_t frameCount{0};
    DurationType realTime{};
    DurationType realDeltaTime{};
    DurationType time{};
    DurationType deltaTime{};
    float timeScale{1.0f};
};
