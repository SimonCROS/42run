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

static auto makeGlBuffer(Engine & engine, const Buffer * buffers, BufferView & bufferView) -> GLuint
{
    if (bufferView.glBuffer.has_value() == 0)
    {
        GLuint glBuffer = 0;

        const auto & buffer = buffers[bufferView.buffer];

        glGenBuffers(1, &glBuffer);
        engine.bindBuffer(bufferView.target, glBuffer);
        glBufferData(bufferView.target, bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset,
                     GL_STATIC_DRAW);

        bufferView.glBuffer = glBuffer;
        return glBuffer;
    }
    else
    {
        return *bufferView.glBuffer;
    }
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
    std::vector<GLuint> textures;
    std::vector<Animation> animations;
    ModelRenderInfo renderInfo;

    textures.resize(model.textures.size(), 0);

    renderInfo.nodesCount = model.nodes.size();
    if (renderInfo.nodesCount > 0)
    {
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
                    trs.rotation = glm::quat(
                        static_cast<float>(node.rotation[3]),
                        static_cast<float>(node.rotation[0]),
                        static_cast<float>(node.rotation[1]),
                        static_cast<float>(node.rotation[2]));
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

    renderInfo.buffersCount = model.buffers.size();
    if (renderInfo.buffersCount > 0)
    {
        renderInfo.buffers = std::make_unique<Buffer[]>(renderInfo.buffersCount);
        for (size_t i = 0; i < renderInfo.buffersCount; ++i)
        {
            renderInfo.buffers[i].data = model.buffers[i].data;
        }
    }

    renderInfo.bufferViewsCount = model.bufferViews.size();
    if (renderInfo.bufferViewsCount > 0)
    {
        renderInfo.bufferViews = std::make_unique<BufferView[]>(renderInfo.bufferViewsCount);
        for (size_t i = 0; i < renderInfo.bufferViewsCount; ++i)
        {
            const auto & bufferView = model.bufferViews[i];
            auto & bufferViewRenderInfo = renderInfo.bufferViews[i];

            bufferViewRenderInfo.buffer = bufferView.buffer;
            bufferViewRenderInfo.target = bufferView.target;
            bufferViewRenderInfo.byteOffset = bufferView.byteOffset;
            bufferViewRenderInfo.byteLength = static_cast<GLsizeiptr>(bufferView.byteLength);
            bufferViewRenderInfo.byteStride = bufferView.byteStride;
        }
    }

    renderInfo.accessorsCount = model.accessors.size();
    if (renderInfo.accessorsCount > 0)
    {
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
            accessorRenderInfo.byteOffset = accessor.byteOffset;
            accessorRenderInfo.bufferView = accessor.bufferView;
            accessorRenderInfo.componentType = accessor.componentType;
            accessorRenderInfo.type = accessor.type;
            accessorRenderInfo.normalized = accessor.normalized;
        }
    }

    renderInfo.skinsCount = model.skins.size();
    if (renderInfo.skinsCount > 0)
    {
        renderInfo.skins = std::make_unique<SkinRenderInfo[]>(renderInfo.skinsCount);
        for (size_t i = 0; i < renderInfo.skinsCount; ++i)
        {
            const auto & skin = model.skins[i];

            auto & skinRenderInfo = renderInfo.skins[i];

            if (skin.inverseBindMatrices > -1)
            {
                const auto & accessor = renderInfo.accessors[skin.inverseBindMatrices];
                const auto & bufferView = renderInfo.bufferViews[accessor.bufferView];
                const auto & buffer = renderInfo.buffers[bufferView.buffer];
                const auto attributeStride = accessor.byteStride / sizeof(glm::mat4);

                StridedIterator it{
                    reinterpret_cast<const glm::mat4 *>(
                        buffer.data.data() + bufferView.byteOffset + accessor.byteOffset),
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

    renderInfo.meshesCount = model.meshes.size();
    if (renderInfo.meshesCount > 0)
    {
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

                if (primitive.indices >= 0)
                {
                    makeGlBuffer(engine, renderInfo.buffers.get(),
                                 renderInfo.bufferViews[renderInfo.accessors[primitive.indices].bufferView]);
                }

                primitiveRenderInfo.attributes.reserve(primitive.attributes.size());
                for (const auto & [attributeName, accessorId]: primitive.attributes)
                {
                    makeGlBuffer(engine, renderInfo.buffers.get(),
                                 renderInfo.bufferViews[renderInfo.accessors[accessorId].bufferView]);

                    PrimitiveAttributeType type{PrimitiveAttributeType::Invalid};

                    if (attributeName == "POSITION")
                    {
                        vertexArrayFlags |= VertexArrayHasPosition;
                        type = PrimitiveAttributeType::Position;
                    }
                    else if (attributeName == "NORMAL")
                    {
                        vertexArrayFlags |= VertexArrayHasNormals;
                        type = PrimitiveAttributeType::Normal;
                    }
                    else if (attributeName == "TANGENT")
                    {
                        vertexArrayFlags |= VertexArrayHasTangents;
                        type = PrimitiveAttributeType::Tangent;
                    }
                    else if (attributeName == "COLOR_0")
                    {
                        vertexArrayFlags |= VertexArrayHasColor0;
                        type = PrimitiveAttributeType::Color0;
                    }
                    else if (attributeName == "TEXCOORD_0")
                    {
                        vertexArrayFlags |= VertexArrayHasTexCoord0;
                        type = PrimitiveAttributeType::TexCoord0;
                    }
                    else if (attributeName == "TEXCOORD_1")
                    {
                        vertexArrayFlags |= VertexArrayHasTexCoord1;
                        type = PrimitiveAttributeType::TexCoord1;
                    }
                    else if (attributeName == "JOINTS_0")
                    {
                        vertexArrayFlags |= VertexArrayHasJoints;
                        type = PrimitiveAttributeType::Joints0;
                    }
                    else if (attributeName == "WEIGHTS_0")
                    {
                        vertexArrayFlags |= VertexArrayHasWeights0;
                        type = PrimitiveAttributeType::Weights0;
                    }

                    primitiveRenderInfo.attributes.emplace_back(type, accessorId);
                }

                primitiveRenderInfo.material = primitive.material;
                primitiveRenderInfo.mode = primitive.mode;
                primitiveRenderInfo.indices = primitive.indices;
                primitiveRenderInfo.vertexArrayFlags = vertexArrayFlags;
            }
        }
    }

    renderInfo.materialsCount = model.materials.size();
    if (renderInfo.materialsCount > 0)
    {
        renderInfo.materials = std::make_unique<Material[]>(renderInfo.materialsCount);
        for (size_t i = 0; i < renderInfo.materialsCount; ++i)
        {
            auto & gltfMaterial = model.materials[i];
            auto & material = renderInfo.materials[i];

            material.pbr.baseColorTexture.index = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
            material.pbr.baseColorTexture.texCoord = gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord;
            material.pbr.baseColorFactor = glm::make_vec4(gltfMaterial.pbrMetallicRoughness.baseColorFactor.data());
            material.pbr.metallicRoughnessTexture.index = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.
                    index;
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

            if (material.pbr.baseColorTexture.index >= 0)
            {
                loadTexture(model, material.pbr.baseColorTexture.index, textures, GL_SRGB_ALPHA);
                material.shaderFlags |= ShaderFlags::HasBaseColorMap;
            }
            if (material.pbr.metallicRoughnessTexture.index >= 0)
            {
                loadTexture(model, material.pbr.metallicRoughnessTexture.index, textures, GL_RGB);
                material.shaderFlags |= ShaderFlags::HasMetalRoughnessMap;
            }
            if (material.normalTexture.index >= 0)
            {
                loadTexture(model, material.normalTexture.index, textures, GL_RGB);
                material.shaderFlags |= ShaderFlags::HasNormalMap;
            }
            if (material.emissiveTexture.index >= 0)
            {
                loadTexture(model, material.emissiveTexture.index, textures, GL_SRGB);
                material.shaderFlags |= ShaderFlags::HasEmissiveMap;
            }
        }
    }

    animations.reserve(model.animations.size());
    for (const auto & animation: model.animations)
    {
        animations.emplace_back(Animation::Create(renderInfo, animation));
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
        std::move(textures), std::move(animations), std::move(renderInfo)
    };
}

auto Model::prepareShaderPrograms(ShaderManager & manager, const SlotSetIndex vertexShaderFile,
    const SlotSetIndex fragmentShaderFile) -> std::expected<void, std::string> {
    auto & renderInfo = m_renderInfo;
    for (int meshIdx = 0; meshIdx < renderInfo.meshesCount; ++meshIdx)
    {
        auto & mesh = renderInfo.meshes[meshIdx];
        for (int primitiveIdx = 0; primitiveIdx < mesh.primitivesCount; ++primitiveIdx)
        {
            auto & primitive = mesh.primitives[primitiveIdx];

            ShaderFlags shaderFlags = [&renderInfo, &primitive]()
            {
                if (primitive.material == -1)
                {
                    return ShaderFlags::None;
                }
                return renderInfo.materials[primitive.material].shaderFlags;
            }();

            if ((primitive.vertexArrayFlags & VertexArrayHasJoints) == VertexArrayHasJoints)
            {
                shaderFlags |= ShaderFlags::HasSkin;
            }

            const auto e_result = manager.
                    getOrCreateShaderProgram(vertexShaderFile, fragmentShaderFile, shaderFlags);
            if (!e_result)
            {
                return std::unexpected(std::move(e_result).error());
            }

            primitive.programIndex = *e_result;
        }
    }

    return {};
}
