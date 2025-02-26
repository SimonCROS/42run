//
// Created by Simon Cros on 1/29/25.
//

module;

#include <optional>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "OpenGL/ShaderProgram.h"

export module Components:MeshRenderer;
import :Animator;
import Engine;

export class MeshRenderer final : public Component
{
private:
    struct Node
    {
        glm::mat4 globalTransform = glm::identity<glm::mat4>();
    };

    struct Skin
    {
        std::vector<glm::mat4> jointMatrices;
    };

    const Mesh& m_mesh;
    bool m_displayed{true};
    GLenum m_polygonMode{GL_FILL};
    std::optional<std::reference_wrapper<const Animator>> m_animator;
    ShaderProgram& m_program; // TODO Change

    std::vector<Node> m_nodes;
    std::vector<Skin> m_skins;

    auto renderMesh(Engine& engine, int meshIndex, const glm::mat4& transform) -> void;
    auto renderNodeRecursive(Engine& engine, int nodeIndex) -> void;
    auto calculateGlobalTransformsRecursive(int nodeIndex, glm::mat4 transform) -> void;
    auto calculateJointMatrices(int skin, const glm::mat4& transform) -> void;

public:
    explicit MeshRenderer(Object& object, const Mesh& model, ShaderProgram& program) :
        Component(object), m_mesh(model), m_program(program)
    {
        m_nodes.resize(m_mesh.model().nodes.size());
        m_skins.resize(m_mesh.model().skins.size());

        for (int i = 0; i < m_skins.size(); ++i)
            m_skins[i].jointMatrices.resize(m_mesh.model().skins[i].joints.size());

        // maybe make Create static function
        auto e_prepareResult = m_mesh.prepareShaderPrograms(program);
        if (!e_prepareResult)
            throw std::runtime_error("Failed to prepare shader programs: " + e_prepareResult.error());
    }

    [[nodiscard]] auto mesh() const -> const Mesh& { return m_mesh; }

    auto setAnimator(const Animator& animator) -> void { m_animator = animator; }
    auto unsetAnimator() -> void { m_animator = std::nullopt; }

    [[nodiscard]] auto displayed() const -> bool
    {
        return m_displayed;
    }

    auto setDisplay(const bool display) -> void
    {
        m_displayed = display;
    }

    [[nodiscard]] auto polygonMode() const noexcept -> GLenum { return m_polygonMode; }

    auto setPolygoneMode(const GLenum polygonMode) -> void { m_polygonMode = polygonMode; }

    auto onRender(Engine& engine) -> void override;
};
