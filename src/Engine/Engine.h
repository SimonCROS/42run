//
// Created by Simon Cros on 1/13/25.
//

#ifndef ENGINE_H
#define ENGINE_H
#include <iostream>
#include <functional>
#include <unordered_set>

#include "FrameInfo.h"
#include "Object.h"
#include "glad/gl.h"
#include "tiny_gltf.h"
#include "OpenGL/ShaderProgram.h"
#include "OpenGL/VertexArray.h"
#include "Utility/SlotSet.h"
#include "Window/Window.h"

class Camera;
class Mesh;

class Engine
{
public:
    using ModelRef = std::reference_wrapper<Mesh>;
    using ObjectRef = std::reference_wrapper<Object>;
    using ShaderProgramVariantsRef = std::reference_wrapper<ShaderProgram>;

    using ModelPtr = std::unique_ptr<Mesh>;
    using ShaderProgramPtr = std::unique_ptr<ShaderProgram>;

    static constexpr size_t MaxTextures = 8;

private:
    Window m_window;
    tinygltf::TinyGLTF m_loader;

    ClockType m_clock{};
    TimePoint m_start{};

    FrameInfo m_currentFrameInfo{};

    StringUnorderedMap<ModelPtr> m_models;
    StringUnorderedMap<ShaderProgramPtr> m_shaders;
    SlotSet<Object> m_objects;
    std::unordered_map<VertexArrayFlags, VertexArray> m_vertexArrays;

    bool m_doubleSided{false};
    bool m_blendEnabled{false};
    GLenum m_polygonMode{GL_FILL};
    GLuint m_currentShaderProgram{0};
    GLuint m_currentTextures[MaxTextures]{};
    GLenum m_currentBoundTextureTarget{0};
    GLuint m_currentBoundVertexArray{0};
    GLuint m_currentBoundArrayBuffer{0};
    GLuint m_currentBoundArrayElementBuffer{0};

    const Camera* m_camera{nullptr};

public:
    static auto Create(Window&& window) -> Engine;

    explicit Engine(Window&& window) noexcept;

    [[nodiscard]] auto getWindow() noexcept -> Window& { return m_window; }
    [[nodiscard]] auto getWindow() const noexcept -> const Window& { return m_window; }

    [[nodiscard]] auto frameInfo() const noexcept -> FrameInfo { return m_currentFrameInfo; }

    [[nodiscard]] auto controls() const noexcept -> Controls { return m_window.getCurrentControls(); }

    [[nodiscard]] auto isDoubleSided() const noexcept -> bool { return m_doubleSided; }
    [[nodiscard]] auto polygonMode() const noexcept -> GLenum { return m_polygonMode; }

    auto run() -> void;

    auto setDoubleSided(const bool value) -> void
    {
        if (m_doubleSided != value)
        {
            m_doubleSided = value;
            if (value)
                glDisable(GL_CULL_FACE);
            else
                glEnable(GL_CULL_FACE);
        }
    }

    auto setBlendEnabled(const bool value) -> void
    {
        if (m_blendEnabled != value)
        {
            m_blendEnabled = value;
            if (value)
                glEnable(GL_BLEND);
            else
                glDisable(GL_BLEND);
        }
    }

    auto setPolygoneMode(const GLenum polygonMode) -> void
    {
        if (m_polygonMode != polygonMode)
        {
            m_polygonMode = polygonMode;
            glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
        }
    }

    auto useProgram(const ShaderProgramInstance& program) -> void
    {
        if (m_currentShaderProgram != program.id())
        {
            m_currentShaderProgram = program.id();
            program.use();
        }
    }

    auto bindVertexArray(const VertexArray& vertexArray) -> void
    {
        if (m_currentBoundVertexArray != vertexArray.id())
        {
            m_currentBoundVertexArray = vertexArray.id();
            vertexArray.bind();
        }
    }

    auto bindBuffer(const GLenum target, const GLuint id) -> void
    {
        if (target == GL_ARRAY_BUFFER && m_currentBoundArrayBuffer != id)
        {
            glBindBuffer(GL_ARRAY_BUFFER, id);
            m_currentBoundArrayBuffer = id;
        }
        else if (target == GL_ELEMENT_ARRAY_BUFFER && m_currentBoundArrayElementBuffer != id)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
            m_currentBoundArrayElementBuffer = id;
        }
        else
        {
            glBindBuffer(target, id);
        }
    }

    auto bindTexture(const GLuint bindingIndex, const GLuint& texture) -> void
    {
        assert(bindingIndex < MaxTextures);
        if (m_currentTextures[bindingIndex] != texture)
        {
            const GLenum target = GL_TEXTURE0 + bindingIndex;

            if (m_currentBoundTextureTarget != target)
            {
                glActiveTexture(target);
                m_currentBoundTextureTarget = target;
            }

            glBindTexture(GL_TEXTURE_2D, texture);
            m_currentTextures[bindingIndex] = texture;
        }
    }

    auto getVertexArray(const VertexArrayFlags flags) -> VertexArray&
    {
        return m_vertexArrays[flags];
    }

    [[nodiscard]]
    auto
    makeShaderVariants(const std::string_view& id, const std::string& vertPath, const std::string& fragPath)
        -> Expected<ShaderProgramVariantsRef, std::string>;

    [[nodiscard]]
    auto
    loadModel(const std::string_view& id, const std::string& path, bool binary)
        -> Expected<ModelRef, std::string>;

    [[nodiscard]]
    auto
    instantiate()
        -> Object&;

    auto setCamera(const Camera& camera) -> void { m_camera = &camera; }
};

#endif //ENGINE_H
