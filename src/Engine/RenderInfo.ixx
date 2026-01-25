//
// Created by scros on 11/29/25.
//

module;

#include "glad/gl.h"

export module Engine.RenderInfo;

import std;
import glm;
import OpenGL;
import Utility.SlotSet;

export using BufferIndex = int;
export using BufferViewIndex = int;
export using AccessorIndex = int;
export using MaterialIndex = int;
export using TextureIndex = int;
export using NodeIndex = int;

export struct Buffer
{
    std::vector<unsigned char> data;
};

export struct BufferView
{
    std::optional<GLint> glBuffer;
    BufferIndex buffer;
    int target;
    size_t byteOffset{0};
    GLsizeiptr byteLength{0};
};

export struct AccessorRenderInfo
{
    GLint componentSize{0};
    GLint componentCount{0};
    GLsizei byteStride{0};
    size_t count{};
    size_t byteOffset{};
    BufferViewIndex bufferView{};
    int componentType{-1};
    int type{-1};
    bool normalized{false};
};

export struct TextureInfo
{
    TextureIndex index;
    int texCoord;
};

export struct NormalTextureInfo
{
    TextureIndex index;
    int texCoord;
    float scale;
};

export struct Material
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
    ShaderFlags shaderFlags{ShaderFlags::None};
    bool doubleSided;
    bool blend; // based on alphaMode, it's a boolean because MASK is not supported
};

export enum class PrimitiveAttributeType
{
    Position = 0,
    Normal = 1,
    Color0 = 2,
    TexCoord0 = 3,
    TexCoord1 = 4,
    Tangent = 5,
    Joints0 = 6,
    Weights0 = 7,
    Invalid = -1,
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
    SlotSetIndex programIndex;
};

export struct MeshRenderInfo
{
    size_t primitivesCount{0};
    std::unique_ptr<PrimitiveRenderInfo[]> primitives{nullptr};
};

export struct SkinRenderInfo
{
    std::vector<glm::mat4> inverseBindMatrices{glm::identity<glm::mat4>()};
    std::vector<int> joints;
    GLuint glBuffer{0};
    int skeleton{-1};
};

export struct TRS
{
    glm::quat rotation{glm::identity<glm::quat>()};
    glm::vec3 translation{};
    glm::vec3 scale{1.0f};
};

export struct NodeRenderInfo
{
    int skin{-1};
    int mesh{-1};
    size_t childrenCount{0};
    std::variant<glm::mat4, TRS> transform{std::in_place_index<1>};
    std::unique_ptr<NodeIndex[]> children{nullptr};
};

export struct ModelRenderInfo
{
    size_t buffersCount{0};
    size_t bufferViewsCount{0};
    size_t accessorsCount{0};
    size_t meshesCount{0};
    size_t skinsCount{0};
    size_t nodesCount{0};
    size_t rootNodesCount{0};
    size_t materialsCount{0};
    std::unique_ptr<Buffer[]> buffers{nullptr};
    std::unique_ptr<BufferView[]> bufferViews{nullptr};
    std::unique_ptr<AccessorRenderInfo[]> accessors{nullptr};
    std::unique_ptr<MeshRenderInfo[]> meshes{nullptr};
    std::unique_ptr<SkinRenderInfo[]> skins{nullptr};
    std::unique_ptr<NodeRenderInfo[]> nodes{nullptr};
    std::unique_ptr<NodeIndex[]> rootNodes{nullptr};
    std::unique_ptr<Material[]> materials{nullptr};
};
