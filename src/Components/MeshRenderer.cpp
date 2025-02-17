//
// Created by Simon Cros on 1/29/25.
//

#include "MeshRenderer.h"
#include "Engine/Engine.h"
#include "Engine/Object.h"
#include "glm/gtc/type_ptr.hpp"

static void* bufferOffset(const size_t offset)
{
    return reinterpret_cast<void*>(offset);
}

auto MeshRenderer::renderMesh(Engine& engine, const int meshIndex, const glm::mat4& transform) -> void
{
    const auto& mesh = m_mesh.model().meshes[meshIndex];
    const auto& meshRenderInfo = m_mesh.renderInfo().meshes[meshIndex];

    for (int p = 0; p < mesh.primitives.size(); ++p)
    {
        const auto& primitive = mesh.primitives[p];
        const auto& primitiveRenderInfo = meshRenderInfo.primitives[p];

        auto& program = m_program.get().getProgram(primitiveRenderInfo.shaderFlags);
        engine.useProgram(program);

        auto& vertexArray = engine.getVertexArray(primitiveRenderInfo.vertexArrayFlags);
        engine.bindVertexArray(vertexArray);

        for (const auto& [attribute, accessorIndex] : primitive.attributes)
        {
            const auto& accessor = m_mesh.model().accessors[accessorIndex];
            const auto& accessorRenderInfo = m_mesh.renderInfo().accessors[accessorIndex];

            const int attributeLocation = VertexArray::getAttributeLocation(attribute);
            if (attributeLocation != -1)
            {
                engine.bindBuffer(GL_ARRAY_BUFFER, accessorRenderInfo.glBuffer);
                glVertexAttribPointer(attributeLocation,
                                      accessorRenderInfo.componentCount,
                                      accessor.componentType,
                                      accessor.normalized,
                                      accessorRenderInfo.byteStride,
                                      bufferOffset(accessorRenderInfo.byteOffsetFromBufferView));
            }
        }

        program.setMat4("u_transform", transform);

        if (primitive.material >= 0)
        {
            const auto& material = m_mesh.model().materials[primitive.material];

            engine.setDoubleSided(material.doubleSided);

            if (material.alphaMode == "BLEND")
                glEnable(GL_BLEND);
            else
                glDisable(GL_BLEND);

            if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
            {
                engine.bindTexture(0, m_mesh.texture(material.pbrMetallicRoughness.baseColorTexture.index));
                program.setInt("u_baseColorTexture", 0);
            }

            if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
            {
                engine.bindTexture(1, m_mesh.texture(material.pbrMetallicRoughness.metallicRoughnessTexture.index));
                program.setInt("u_metallicRoughnessMap", 1);
            }

            if (material.normalTexture.index >= 0)
            {
                engine.bindTexture(2, m_mesh.texture(material.normalTexture.index));
                program.setInt("u_normalMap", 2);
            }

            if (material.emissiveTexture.index >= 0)
            {
                engine.bindTexture(3, m_mesh.texture(material.emissiveTexture.index));
                program.setInt("u_emissiveMap", 3);
            }

            program.setVec4("u_baseColorFactor", glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data()));
            program.setFloat("u_metallicFactor", static_cast<float>(material.pbrMetallicRoughness.metallicFactor));
            program.setFloat("u_roughnessFactor", static_cast<float>(material.pbrMetallicRoughness.roughnessFactor));
            program.setFloat("u_normalScale", static_cast<float>(material.normalTexture.scale));
            program.setVec3("u_emissiveFactor", glm::make_vec4(material.emissiveFactor.data()));
        }
        else
        {
            engine.setDoubleSided(false);
        }

        assert(primitive.indices >= 0); // TODO handle non indexed primitives

        const auto& indexAccessor = m_mesh.model().accessors[primitive.indices];
        const auto& indexAccessorRenderInfo = m_mesh.renderInfo().accessors[primitive.indices];

        engine.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexAccessorRenderInfo.glBuffer);

        glDrawElements(primitive.mode, static_cast<GLsizei>(indexAccessorRenderInfo.count), indexAccessor.componentType,
                       bufferOffset(indexAccessorRenderInfo.byteOffsetFromBufferView));
    }
}

auto MeshRenderer::renderNodeRecursive(Engine& engine, const int nodeIndex) -> void
{
    const tinygltf::Node& node = m_mesh.model().nodes[nodeIndex];

    if (node.mesh > -1)
        renderMesh(engine, node.mesh, m_nodes[nodeIndex].globalTransform);
    for (const auto childIndex : node.children)
        renderNodeRecursive(engine, childIndex);
}

auto MeshRenderer::calculateGlobalTransformsRecursive(const int nodeIndex, glm::mat4 transform) -> void
{
    const tinygltf::Node& node = m_mesh.model().nodes[nodeIndex];

    // TODO Precalculate matrix or trs
    if (!node.matrix.empty())
    {
        transform *= glm::mat4(node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
                               node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
                               node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
                               node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]);
    }
    else
    {
        const auto& tr = m_animator.has_value()
                             ? m_animator->get().nodeTransform(nodeIndex)
                             : Animator::AnimatedTransform{};

        if (tr.translation.has_value())
            transform = glm::translate(transform, *tr.translation);
        else if (!node.translation.empty())
            transform = glm::translate(
                transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));

        if (tr.rotation.has_value())
            transform *= glm::mat4_cast(*tr.rotation);
        else if (!node.rotation.empty())
            transform *= glm::mat4_cast(glm::quat(node.rotation[3], node.rotation[0], node.rotation[1],
                                                  node.rotation[2]));

        if (tr.scale.has_value())
            transform = glm::scale(transform, *tr.scale);
        else if (!node.scale.empty())
            transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
    }

    m_nodes[nodeIndex].globalTransform = transform;

    for (const auto childIndex : node.children)
        calculateGlobalTransformsRecursive(childIndex, transform);
}

auto MeshRenderer::calculateJointMatrices(const int skinIndex, const glm::mat4& transform) -> void
{
    const auto& gltfSkin = m_mesh.model().skins[skinIndex];
    const auto& gltfJoints = gltfSkin.joints;
    auto& jointMatrices = m_skins[skinIndex].jointMatrices;

    glm::mat4 globalInverseTransform;
    if (gltfSkin.skeleton > -1)
        globalInverseTransform = glm::inverse(m_nodes[gltfSkin.skeleton].globalTransform);
    else
        globalInverseTransform = transform;

    if (gltfSkin.inverseBindMatrices != -1)
    {
        for (int i = 0; i < gltfJoints.size(); ++i)
            jointMatrices[i] = globalInverseTransform * m_nodes[gltfJoints[i]].globalTransform * m_mesh.renderInfo().
                skins[skinIndex].inverseBindMatrices[i];
    }
    else
    {
        for (int i = 0; i < gltfJoints.size(); ++i)
            jointMatrices[i] = globalInverseTransform * m_nodes[gltfJoints[i]].globalTransform;
    }
}

void MeshRenderer::onRender(Engine& engine)
{
    if (!displayed())
        return;

    if (engine.polygonMode() != m_polygonMode)
        engine.setPolygoneMode(m_polygonMode);

    glm::mat4 globalTransform = object().transform().trs();

    for (const auto nodeIndex : m_mesh.model().scenes[m_mesh.model().defaultScene].nodes)
        calculateGlobalTransformsRecursive(nodeIndex, globalTransform);

    for (int skinIndex = 0; skinIndex < m_mesh.model().skins.size(); ++skinIndex)
    {
        calculateJointMatrices(skinIndex, globalTransform);

        const auto jointsUBO = m_mesh.renderInfo().skins[skinIndex].glBuffer;
        const GLuint uniformBlockBinding = skinIndex;
        glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockBinding, jointsUBO);

        for (auto& program : m_program.get().programs)
        {
            if (program.first & ShaderHasSkin)
            {
                program.second->setUniformBlock("JointMatrices", uniformBlockBinding);
            }
        }

        glBindBuffer(GL_UNIFORM_BUFFER, jointsUBO);
        glBufferSubData(
            GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(m_skins[skinIndex].jointMatrices.size() * sizeof(glm::mat4)),
            m_skins[skinIndex].jointMatrices.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    for (const auto nodeIndex : m_mesh.model().scenes[m_mesh.model().defaultScene].nodes)
        renderNodeRecursive(engine, nodeIndex);
}
