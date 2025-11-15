//
// Created by Simon Cros on 26/01/2025.
//

module;

#include "glm/glm.hpp"
#include "glad/gl.h"
#include "tiny_gltf.h"

export module Engine:Mesh;
import std;
import Engine.Animation;
import :Engine;
import OpenGL;

export using AccessorIndex = int;
export using MaterialIndex = int;
export using TextureIndex = int;

struct TextureInfo
{
    TextureIndex index;
    int texCoord;
};

struct NormalTextureInfo
{
    TextureIndex index;
    int texCoord;
    float scale;
};

struct Material
{
    struct PBR
    {
        TextureInfo baseColorTexture;
        glm::vec4 baseColorFactor;
        TextureInfo metallicRoughnessTexture;
        float metallicFactor;
        float roughnessFactor;
    };

    PBR pbr;
    NormalTextureInfo normalTexture;
    TextureInfo emissiveTexture;
    glm::vec3 emissiveFactor;
    bool doubleSided;
    bool blend; // based on alphaMode, it's a boolean because MASK is not supported
};

export struct AccessorRenderInfo
{
    GLuint glBuffer{0};
    GLint componentSize{0};
    GLint componentCount{0};
    GLsizei byteStride{0};
    size_t count;
    size_t byteOffsetFromBuffer;
    size_t byteOffsetFromBufferView;
    int bufferView{};
    int componentType{-1};
    bool normalized{false};
};

export enum class PrimitiveAttributeType
{
    Position,
    Normal,
    Tangent,
    Color0,
    TexCoord0,
    TexCoord1,
    Joints0,
    Weights0,
    Invalid,
};

export struct PrimitiveAttribute
{
    PrimitiveAttributeType type;
    AccessorIndex accessor;
};

export struct PrimitiveRenderInfo
{
    std::vector<PrimitiveAttribute> attributes;
    int material{-1};
    int mode{-1};
    AccessorIndex indices{-1};
    VertexArrayFlags vertexArrayFlags{VertexArrayHasNone};
    ShaderFlags shaderFlags{ShaderHasNone};
};

export struct MeshRenderInfo
{
    size_t primitivesCount{0};
    std::unique_ptr<PrimitiveRenderInfo[]> primitives{nullptr};
};

export struct SkinRenderInfo
{
    GLuint glBuffer{0};
    std::vector<glm::mat4> inverseBindMatrices{};
};

export struct ModelRenderInfo
{
    size_t accessorsCount{0};
    size_t meshesCount{0};
    size_t skinsCount{0};
    std::unique_ptr<AccessorRenderInfo[]> accessors{nullptr};
    std::unique_ptr<MeshRenderInfo[]> meshes{nullptr};
    std::unique_ptr<SkinRenderInfo[]> skins{nullptr};
};

export class Model
{
private:
    std::vector<GLuint> m_buffers;
    std::vector<GLuint> m_textures;
    std::vector<Animation> m_animations; // TODO use a pointer to ensure location never change and faster access
    std::vector<Material> m_materials;
    ModelRenderInfo m_renderInfo;

public:
    static auto Create(Engine & engine, const tinygltf::Model & model) -> Model;

    Model(std::vector<GLuint> && buffers, std::vector<GLuint> && textures, std::vector<Animation> && animations,
          std::vector<Material> && materials, ModelRenderInfo && renderInfo) : m_buffers(std::move(buffers)),
                                                                               m_textures(std::move(textures)),
                                                                               m_animations(std::move(animations)),
                                                                               m_materials(std::move(materials)),
                                                                               m_renderInfo(std::move(renderInfo))
    {}

    [[nodiscard]] auto buffer(const size_t index) const -> GLuint { return m_buffers[index]; }

    [[nodiscard]] auto texture(const size_t index) const -> GLuint { return m_textures[index]; }

    [[nodiscard]] auto animations() const -> const std::vector<Animation> & { return m_animations; }

    [[nodiscard]] auto materials() const -> const std::vector<Material> & { return m_materials; }

    [[nodiscard]] auto renderInfo() const -> const ModelRenderInfo & { return m_renderInfo; }

    [[nodiscard]] auto prepareShaderPrograms(ShaderProgram & builder) const -> std::expected<void, std::string>
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
