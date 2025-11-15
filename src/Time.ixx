//
// Created by scros on 11/15/25.
//

export module Time;
import std;

export using ClockType = std::chrono::steady_clock;
export using TimePoint = ClockType::time_point;
export using DurationType = std::chrono::duration<float>;
