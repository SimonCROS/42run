//
// Created by Simon Cros on 1/29/25.
//

#ifndef MODELRENDERER_H
#define MODELRENDERER_H
#include "Animator.h"
#include "Engine/EngineComponent.h"
#include "Engine/Mesh.h"
#include "OpenGL/ShaderProgram.h"

class MeshRenderer final : public EngineComponent
{
private:
    const Mesh& m_mesh;
    bool m_displayed{true};
    GLenum m_polygonMode{GL_FILL};
    std::optional<std::reference_wrapper<const Animator>> m_animator;
    std::vector<glm::vec3> m_scaleMultiplier;

    std::reference_wrapper<ShaderProgram>& m_program; // TODO Change

    auto renderMesh(Engine& engine, int meshIndex, const glm::mat4& transform) -> void;
    auto renderNode(Engine& engine, int nodeIndex, glm::mat4 transform) -> void;

public:
    explicit MeshRenderer(Object& object, const Mesh& model, std::reference_wrapper<ShaderProgram>& program) :
        EngineComponent(object), m_mesh(model), m_program(program)
    {
        m_scaleMultiplier.resize(m_mesh.model().nodes.size(), glm::vec3(1));

        // maybe make Create static function
        auto e_prepareResult = m_mesh.prepareShaderPrograms(program);
        if (!e_prepareResult)
            throw std::runtime_error("Failed to prepare shader programs: " + e_prepareResult.error());
    }

    [[nodiscard]] auto mesh() const -> const Mesh& { return m_mesh; }

    auto setAnimator(const Animator& animator) -> void { m_animator = animator; }
    auto unsetAnimator() -> void { m_animator = std::nullopt; }

    auto setScaleMultiplier(const size_t nodeIndex, const glm::vec3 scale) -> void
    {
        m_scaleMultiplier[nodeIndex] = scale;
    }

    [[nodiscard]] auto getScaleMultiplier(const size_t nodeIndex) const -> glm::vec3
    {
        return m_scaleMultiplier[nodeIndex];
    }

    [[nodiscard]] auto displayed() const -> bool
    {
        return m_displayed;
    }

    auto setDisplay(const bool display) -> void
    {
        m_displayed = display;
    }

    [[nodiscard]] auto polygonMode() const noexcept -> GLenum { return m_polygonMode; }

    auto setPolygoneMode(Engine &engine, const GLenum polygonMode) -> void
    {
        m_polygonMode = polygonMode;
        if (engine.polygonMode() != m_polygonMode)
            engine.setPolygoneMode(polygonMode);
    }

    auto onRender(Engine& engine) -> void override;
};

#endif //MODELRENDERER_H
