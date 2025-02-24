//
// Created by Simon Cros on 2/24/25.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <optional>
#include "FrameInfo.h"
#include "OpenGL/ShaderProgram.h"
#include "OpenGL/VertexArray.h"
#include "Utility/SlotSet.h"

class Camera;
class Window;
class Mesh;
class Object;

class Engine
{
public:
    using ModelRef = std::reference_wrapper<Mesh>;
    using ObjectRef = std::reference_wrapper<Object>;
    using ShaderProgramVariantsRef = std::reference_wrapper<ShaderProgram>;

    using ModelPtr = std::unique_ptr<Mesh>;
    using ShaderProgramPtr = std::unique_ptr<ShaderProgram>;

    static constexpr size_t MaxTextures = 8;

    static auto Create(Window&& window) -> Engine;

    explicit Engine(Window&& window) noexcept;

    [[nodiscard]] auto getWindow() noexcept -> Window&;
    [[nodiscard]] auto getWindow() const noexcept -> const Window&;

    [[nodiscard]] auto frameInfo() const noexcept -> FrameInfo;

    // [[nodiscard]] auto controls() const noexcept -> Controls;

    [[nodiscard]] auto isDoubleSided() const noexcept -> bool;
    [[nodiscard]] auto polygonMode() const noexcept -> GLenum;

    auto run() -> void;
    auto setDoubleSided(bool value) -> void;
    auto setBlendEnabled(bool value) -> void;
    auto setPolygoneMode(GLenum polygonMode) -> void;
    auto useProgram(const ShaderProgramInstance& program) -> void;
    auto bindVertexArray(const VertexArray& vertexArray) -> void;
    auto bindBuffer(GLenum target, GLuint id) -> void;
    auto bindTexture(GLuint bindingIndex, const GLuint& texture) -> void;
    auto getVertexArray(VertexArrayFlags flags) -> VertexArray&;

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

    auto setCamera(const Camera& camera) -> void;
    auto objects() -> SlotSet<Object>&;

    [[nodiscard]] auto getShaderProgram(const std::string_view& id) const -> std::optional<std::reference_wrapper<ShaderProgram>>;
    [[nodiscard]] auto getModel(const std::string_view& id) const -> std::optional<std::reference_wrapper<Mesh>>;
};

#endif //ENGINE_H
