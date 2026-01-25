//
// Created by Simon Cros on 1/29/25.
//

module;

#include "glad/gl.h"

export module Components:MeshRenderer;
import std;
import glm;
import :Animator;
import Engine;
import OpenGL;
import OpenGL.Cubemap;
import OpenGL.Texture2D;

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

    const Model& m_mesh;
    bool m_displayed{true};
    GLenum m_polygonMode{GL_FILL};
    std::optional<std::reference_wrapper<const Animator>> m_animator;
    const OpenGL::Cubemap& m_irradianceMap;
    const OpenGL::Cubemap& m_prefilterMap;
    const OpenGL::Texture2D& m_brdfLUT;

    std::vector<Node> m_nodes;
    std::vector<Skin> m_skins;

    auto renderMesh(Engine& engine, int meshIndex, const glm::mat4& transform) -> void;
    auto renderNodeRecursive(Engine& engine, int nodeIndex) -> void;
    auto calculateGlobalTransformsRecursive(int nodeIndex, glm::mat4 transform) -> void;
    auto calculateJointMatrices(int skin, const glm::mat4& transform) -> void;

public:
    explicit MeshRenderer(Object& object, const Model& model, const OpenGL::Cubemap& irradianceMap, const OpenGL::Cubemap& prefilterMap, const OpenGL::Texture2D& brdfLUT) :
        Component(object), m_mesh(model), m_irradianceMap(irradianceMap), m_prefilterMap(prefilterMap), m_brdfLUT(brdfLUT)
    {
        m_nodes.resize(m_mesh.renderInfo().nodesCount);
        m_skins.resize(m_mesh.renderInfo().skinsCount);

        for (int i = 0; i < m_skins.size(); ++i)
        {
            m_skins[i].jointMatrices.resize(m_mesh.renderInfo().skins[i].joints.size());
        }
    }

    [[nodiscard]] auto mesh() const -> const Model& { return m_mesh; }

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
