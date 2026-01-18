//
// Created by Simon Cros on 2/20/25.
//

export module Components:MapController;
import std;
import Engine;
import OpenGL;
import Time;
import OpenGL.Cubemap2;
import OpenGL.Texture2D2;

export class MapController final : public Component
{
private:
    static constexpr float BaseSpeed = 2;
    static constexpr float MaxSpeed = 20;
    static constexpr int MinMovingSegments = 5;
    static constexpr float TMPSegmentSize = 20;
    static constexpr DurationType TimeToReachMaxSpeed = std::chrono::duration_cast<DurationType>(
        std::chrono::seconds(120));

    std::queue<std::reference_wrapper<Object>> m_segmentsPool;
    std::deque<std::reference_wrapper<Object>> m_movingSegments;

    const OpenGL::Cubemap2& m_irradianceMap;
    const OpenGL::Cubemap2& m_prefilterMap;
    const OpenGL::Texture2D2& m_brdfLUT;

    DurationType m_startTime{}; // TODO set in something like onStart

    static constexpr auto easeOutQuad(const float x) -> float
    {
        return 1 - (1 - x) * (1 - x);
    }

public:
    explicit MapController(Object& object, const OpenGL::Cubemap2& irradianceMap, const OpenGL::Cubemap2& prefilterMap, const OpenGL::Texture2D2& brdfLUT) :
        Component(object), m_irradianceMap(irradianceMap), m_prefilterMap(prefilterMap), m_brdfLUT(brdfLUT) {}

    auto onUpdate(Engine& engine) -> void override;
};
