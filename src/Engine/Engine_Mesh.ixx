//
// Created by Simon Cros on 26/01/2025.
//

module;

#include <memory>
#include <vector>
#include <expected>

#include "glm/glm.hpp"
#include "glad/gl.h"
#include "tiny_gltf.h"

export module Engine:Mesh;
import :Animation;
import :Engine;
import OpenGL;

export struct AccessorRenderInfo
{
    GLuint glBuffer{0};
    GLint componentSize{0};
    GLint componentCount{0};
    GLsizei byteStride{0};
    int bufferView{};
    size_t count;
    size_t byteOffsetFromBuffer;
    size_t byteOffsetFromBufferView;
};

export struct PrimitiveRenderInfo
{
    VertexArrayFlags vertexArrayFlags{VertexArrayHasNone};
    ShaderFlags shaderFlags{ShaderHasNone};
};

export struct MeshRenderInfo
{
    std::unique_ptr<PrimitiveRenderInfo[]> primitives{nullptr};
};

export struct SkinRenderInfo
{
    GLuint glBuffer{0};
    std::vector<glm::mat4> inverseBindMatrices{};
};

export struct ModelRenderInfo
{
    std::unique_ptr<AccessorRenderInfo[]> accessors{nullptr};
    std::unique_ptr<MeshRenderInfo[]> meshes{nullptr};
    std::unique_ptr<SkinRenderInfo[]> skins{nullptr};
};

export class Mesh
{
private:
    std::vector<GLuint> m_buffers;
    std::vector<GLuint> m_textures;
    std::vector<Animation> m_animations; // TODO use a pointer to ensure location never change and faster access
    ModelRenderInfo m_renderInfo;

    tinygltf::Model m_model;

public:
    static auto Create(Engine& engine, tinygltf::Model&& model) -> Mesh;

    Mesh(std::vector<GLuint>&& buffers, std::vector<GLuint>&& textures, std::vector<Animation>&& animations,
         ModelRenderInfo&& renderInfo, tinygltf::Model&& model) :
        m_buffers(std::move(buffers)), m_textures(std::move(textures)), m_animations(std::move(animations)),
        m_renderInfo(std::move(renderInfo)), m_model(std::move(model))
    {
    }

    [[nodiscard]] auto model() const -> const tinygltf::Model& { return m_model; }

    [[nodiscard]] auto buffer(const size_t index) const -> GLuint { return m_buffers[index]; }

    [[nodiscard]] auto texture(const size_t index) const -> GLuint { return m_textures[index]; }

    [[nodiscard]] auto animations() const -> const std::vector<Animation>& { return m_animations; }

    [[nodiscard]] auto renderInfo() const -> const ModelRenderInfo& { return m_renderInfo; }

    [[nodiscard]] auto prepareShaderPrograms(ShaderProgram& builder) const -> std::expected<void, std::string>
    {
        for (int i = 0; i < m_model.meshes.size(); ++i)
        {
            for (int j = 0; j < m_model.meshes[i].primitives.size(); ++j)
            {
                auto e_success = builder.enableVariant(m_renderInfo.meshes[i].primitives[j].shaderFlags);
                if (!e_success)
                    return std::unexpected(std::move(e_success).error());
            }
        }

        return {};
    }
};
