//
// Created by Simon Cros on 26/01/2025.
//

module;

#include "42runConfig.h"
#include "tiny_gltf.h"
#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

module Engine;
import std;
import OpenGL;
import Utility;

static auto addBuffer(Engine & engine, const tinygltf::Model & model, const size_t accessorId,
                      std::vector<GLuint> & buffers, ModelRenderInfo & renderInfo) -> GLuint
{
    GLuint glBuffer = 0;

    const auto & accessor = model.accessors[accessorId];
    glBuffer = buffers[accessor.bufferView];
    if (glBuffer == 0)
    {
        const auto & bufferView = model.bufferViews[accessor.bufferView];
        const auto & buffer = model.buffers[bufferView.buffer];

        glGenBuffers(1, &glBuffer);
        engine.bindBuffer(bufferView.target, glBuffer);
        glBufferData(bufferView.target, bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset,
                     GL_STATIC_DRAW);

        buffers[accessor.bufferView] = glBuffer;
    }

    renderInfo.accessors[accessorId].glBuffer = glBuffer;
    return glBuffer;
}

static auto loadTexture(const tinygltf::Model & model, const int & textureId, std::vector<GLuint> & textures,
                        const GLint internalFormat) -> void
{
    if (textures[textureId] > 0)
        return;

    const auto & texture = model.textures[textureId];
    assert(texture.source >= 0);

    GLuint glTexture = 0;
    const auto & image = model.images[texture.source];

    if (!image.image.empty())
    {
        glGenTextures(1, &glTexture);
        glBindTexture(GL_TEXTURE_2D, glTexture);

        if (texture.sampler >= 0)
        {
            const auto & sampler = model.samplers[texture.sampler];
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
            if (sampler.minFilter > -1)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
            }
            if (sampler.magFilter > -1)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
            }
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        GLenum format = GL_RGBA;
        if (image.component == 1)
        {
            format = GL_RED;
        }
        else if (image.component == 2)
        {
            format = GL_RG;
        }
        else if (image.component == 3)
        {
            format = GL_RGB;
        }

        GLenum type = GL_UNSIGNED_BYTE;
        if (image.bits == 16)
        {
            type = GL_UNSIGNED_SHORT;
        }
        else if (image.bits == 32)
        {
            type = GL_UNSIGNED_INT;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.width, image.height, 0, format,
                     type, image.image.data());
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    textures[textureId] = glTexture;
}

auto Model::Create(Engine & engine, const tinygltf::Model & model) -> Model
{
    std::vector<GLuint> buffers;
    std::vector<GLuint> textures;
    std::vector<Animation> animations;
    std::vector<Material> materials;
    ModelRenderInfo renderInfo;

    if (!model.nodes.empty())
    {
        renderInfo.nodesCount = model.nodes.size();
        renderInfo.nodes = std::make_unique<NodeRenderInfo[]>(renderInfo.nodesCount);
        for (size_t i = 0; i < renderInfo.nodesCount; ++i)
        {
            const auto & node = model.nodes[i];

            auto & nodeRenderInfo = renderInfo.nodes[i];

            nodeRenderInfo.skin = node.skin;
            nodeRenderInfo.mesh = node.mesh;
            nodeRenderInfo.childrenCount = node.children.size();
            nodeRenderInfo.children = std::make_unique<NodeIndex[]>(nodeRenderInfo.childrenCount);
            static_assert(std::is_same_v<decltype(node.children)::value_type, NodeIndex>);
            static_assert(std::is_trivially_copyable_v<NodeIndex>);
            std::memcpy(nodeRenderInfo.children.get(), node.children.data(),
                        sizeof(NodeIndex) * nodeRenderInfo.childrenCount);

            if (!node.matrix.empty())
            {
                assert(node.matrix.size() == 16);
                nodeRenderInfo.transform = glm::mat4(node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
                                                     node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
                                                     node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
                                                     node.matrix[12], node.matrix[13], node.matrix[14],
                                                     node.matrix[15]);
            }
            else
            {
                TRS trs{};
                if (!node.translation.empty())
                {
                    assert(node.translation.size() == 3);
                    trs.translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
                }
                if (!node.rotation.empty())
                {
                    assert(node.rotation.size() == 4);
                    trs.rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
                }
                if (!node.scale.empty())
                {
                    assert(node.scale.size() == 3);
                    trs.scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
                }
                nodeRenderInfo.transform = trs;
            }
        }
    }

    if (!model.accessors.empty())
    {
        renderInfo.accessorsCount = model.accessors.size();
        renderInfo.accessors = std::make_unique<AccessorRenderInfo[]>(renderInfo.accessorsCount);
        for (size_t i = 0; i < renderInfo.accessorsCount; ++i)
        {
            const auto & accessor = model.accessors[i];
            const auto & bufferView = model.bufferViews[accessor.bufferView];

            auto & accessorRenderInfo = renderInfo.accessors[i];
            accessorRenderInfo.componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
            accessorRenderInfo.componentCount = tinygltf::GetNumComponentsInType(accessor.type);
            accessorRenderInfo.byteStride = accessor.ByteStride(bufferView);
            accessorRenderInfo.count = accessor.count;
            accessorRenderInfo.byteOffsetFromBuffer = accessor.byteOffset + bufferView.byteOffset;
            accessorRenderInfo.byteOffsetFromBufferView = accessor.byteOffset;
            accessorRenderInfo.bufferView = accessor.bufferView;
            accessorRenderInfo.componentType = accessor.componentType;
            accessorRenderInfo.normalized = accessor.normalized;
        }
    }

    if (!model.skins.empty())
    {
        renderInfo.skinsCount = model.skins.size();
        renderInfo.skins = std::make_unique<SkinRenderInfo[]>(renderInfo.skinsCount);
        for (size_t i = 0; i < renderInfo.skinsCount; ++i)
        {
            const auto & skin = model.skins[i];

            auto & skinRenderInfo = renderInfo.skins[i];

            if (skin.inverseBindMatrices > -1)
            {
                const auto & accessor = renderInfo.accessors[skin.inverseBindMatrices];
                const auto & bufferView = model.bufferViews[accessor.bufferView];
                const auto & buffer = model.buffers[bufferView.buffer];
                const auto attributeStride = accessor.byteStride / sizeof(glm::mat4);

                StridedIterator it{
                    reinterpret_cast<const glm::mat4 *>(buffer.data.data() + accessor.byteOffsetFromBuffer),
                    static_cast<StridedIterator<const glm::mat4 *>::difference_type>(attributeStride),
                };
                skinRenderInfo.inverseBindMatrices = std::vector<glm::mat4>{
                    it, it + static_cast<long>(accessor.count)
                };
            }

            skinRenderInfo.skeleton = skin.skeleton;
            skinRenderInfo.joints = skin.joints;

            glGenBuffers(1, &skinRenderInfo.glBuffer);
            engine.bindBuffer(GL_UNIFORM_BUFFER, skinRenderInfo.glBuffer);
            assert(skinRenderInfo.joints.size() <= MAX_JOINTS && "Too many joints");
            glBufferData(GL_UNIFORM_BUFFER,
                         static_cast<GLsizeiptr>(skinRenderInfo.joints.size() * sizeof(glm::mat4)),
                         nullptr,
                         GL_DYNAMIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
    }

    buffers.resize(model.bufferViews.size(), 0);
    textures.resize(model.textures.size(), 0);

    if (!model.meshes.empty())
    {
        renderInfo.meshesCount = model.meshes.size();
        renderInfo.meshes = std::make_unique<MeshRenderInfo[]>(renderInfo.meshesCount);
        for (size_t i = 0; i < renderInfo.meshesCount; ++i)
        {
            const auto & mesh = model.meshes[i];
            auto & meshRenderInfo = renderInfo.meshes[i];

            meshRenderInfo.primitivesCount = mesh.primitives.size();
            meshRenderInfo.primitives = std::make_unique<PrimitiveRenderInfo[]>(meshRenderInfo.primitivesCount);

            for (size_t j = 0; j < meshRenderInfo.primitivesCount; ++j)
            {
                const auto & primitive = mesh.primitives[j];
                auto & primitiveRenderInfo = meshRenderInfo.primitives[j];

                VertexArrayFlags vertexArrayFlags = VertexArrayHasNone;
                ShaderFlags shaderFlags = ShaderHasNone;

                if (primitive.indices >= 0)
                    addBuffer(engine, model, primitive.indices, buffers, renderInfo);

                primitiveRenderInfo.attributes.reserve(primitive.attributes.size());
                for (const auto & [attributeName, accessorId]: primitive.attributes)
                {
                    addBuffer(engine, model, accessorId, buffers, renderInfo);

                    PrimitiveAttributeType type{PrimitiveAttributeType::Invalid};

                    if (attributeName == "POSITION")
                    {
                        vertexArrayFlags |= VertexArrayHasPosition;
                        type = PrimitiveAttributeType::Position;
                    }
                    else if (attributeName == "NORMAL")
                    {
                        vertexArrayFlags |= VertexArrayHasNormals;
                        shaderFlags |= ShaderHasNormals;
                        type = PrimitiveAttributeType::Normal;
                    }
                    else if (attributeName == "TANGENT")
                    {
                        vertexArrayFlags |= VertexArrayHasTangents;
                        shaderFlags |= ShaderHasTangents;
                        type = PrimitiveAttributeType::Tangent;
                    }
                    else if (attributeName == "COLOR_0")
                    {
                        vertexArrayFlags |= VertexArrayHasColor0;
                        if (model.accessors[accessorId].type == TINYGLTF_TYPE_VEC3)
                        {
                            shaderFlags |= ShaderHasVec3Colors;
                        }
                        else if (model.accessors[accessorId].type == TINYGLTF_TYPE_VEC4)
                        {
                            shaderFlags |= ShaderHasVec4Colors;
                        }
                        type = PrimitiveAttributeType::Color0;
                    }
                    else if (attributeName == "TEXCOORD_0")
                    {
                        vertexArrayFlags |= VertexArrayHasTexCoord0;
                        shaderFlags |= ShaderHasTexCoord0;
                        type = PrimitiveAttributeType::TexCoord0;
                    }
                    else if (attributeName == "TEXCOORD_1")
                    {
                        vertexArrayFlags |= VertexArrayHasTexCoord1;
                        shaderFlags |= ShaderHasTexCoord1;
                        type = PrimitiveAttributeType::TexCoord1;
                    }
                    else if (attributeName == "JOINTS_0")
                    {
                        vertexArrayFlags |= VertexArrayHasSkin;
                        shaderFlags |= ShaderHasSkin;
                        type = PrimitiveAttributeType::Joints0;
                    }
                    else if (attributeName == "WEIGHTS_0")
                    {
                        type = PrimitiveAttributeType::Weights0;
                    }

                    primitiveRenderInfo.attributes.emplace_back(type, accessorId);
                }

                if (primitive.material >= 0)
                {
                    const auto & material = model.materials[primitive.material];
                    if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
                    {
                        loadTexture(model, material.pbrMetallicRoughness.baseColorTexture.index, textures,
                                    GL_SRGB_ALPHA);
                        shaderFlags |= ShaderHasBaseColorMap;
                    }
                    if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
                    {
                        loadTexture(model, material.pbrMetallicRoughness.metallicRoughnessTexture.index, textures,
                                    GL_RGB);
                        shaderFlags |= ShaderHasMetalRoughnessMap;
                    }
                    if (material.normalTexture.index >= 0)
                    {
                        loadTexture(model, material.normalTexture.index, textures, GL_RGB);
                        shaderFlags |= ShaderHasNormalMap;
                    }
                    if (material.emissiveTexture.index >= 0)
                    {
                        loadTexture(model, material.emissiveTexture.index, textures, GL_SRGB);
                        shaderFlags |= ShaderHasEmissiveMap;
                    }
                }

                primitiveRenderInfo.material = primitive.material;
                primitiveRenderInfo.mode = primitive.mode;
                primitiveRenderInfo.indices = primitive.indices;
                primitiveRenderInfo.vertexArrayFlags = vertexArrayFlags;
                primitiveRenderInfo.shaderFlags = shaderFlags;
            }
        }
    }

    animations.reserve(model.animations.size());
    for (const auto & animation: model.animations)
    {
        animations.emplace_back(Animation::Create(model, animation));
    }

    materials.resize(model.materials.size());
    for (int i = 0; i < model.materials.size(); ++i)
    {
        auto & gltfMaterial = model.materials[i];
        auto & material = materials[i];

        material.pbr.baseColorTexture.index = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        material.pbr.baseColorTexture.texCoord = gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord;
        material.pbr.baseColorFactor = glm::make_vec4(gltfMaterial.pbrMetallicRoughness.baseColorFactor.data());
        material.pbr.metallicRoughnessTexture.index = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
        material.pbr.metallicRoughnessTexture.texCoord = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.
                texCoord;
        material.pbr.metallicFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
        material.pbr.roughnessFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
        material.normalTexture.index = gltfMaterial.normalTexture.index;
        material.normalTexture.texCoord = gltfMaterial.normalTexture.texCoord;
        material.normalTexture.scale = static_cast<float>(gltfMaterial.normalTexture.scale);
        material.emissiveTexture.index = gltfMaterial.emissiveTexture.index;
        material.emissiveTexture.texCoord = gltfMaterial.emissiveTexture.texCoord;
        material.emissiveFactor = glm::make_vec3(gltfMaterial.emissiveFactor.data());
        material.doubleSided = gltfMaterial.doubleSided;
        material.blend = gltfMaterial.alphaMode == "BLEND";
    }

    assert(!model.scenes.empty() && "Library GLTF are not supported");
    assert(model.defaultScene != -1 && "A default scene is required");
    const auto & scene = model.scenes[model.defaultScene];

    renderInfo.rootNodesCount = scene.nodes.size();
    renderInfo.rootNodes = std::make_unique<NodeIndex[]>(renderInfo.rootNodesCount);
    static_assert(std::is_same_v<decltype(scene.nodes)::value_type, NodeIndex>);
    static_assert(std::is_trivially_copyable_v<NodeIndex>);
    std::memcpy(renderInfo.rootNodes.get(), scene.nodes.data(), sizeof(NodeIndex) * renderInfo.rootNodesCount);

    return {
        std::move(buffers), std::move(textures), std::move(animations), std::move(materials), std::move(renderInfo)
    };
}
