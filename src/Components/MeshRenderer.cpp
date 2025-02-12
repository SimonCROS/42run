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
                vertexArray.bindArrayBuffer(accessorRenderInfo.bufferId);
                glVertexAttribPointer(attributeLocation,
                                      accessorRenderInfo.componentCount,
                                      accessor.componentType,
                                      GL_FALSE,
                                      accessorRenderInfo.byteStride,
                                      bufferOffset(accessor.byteOffset));
            }
        }

        program.setMat4("u_transform", transform);

        if (primitive.material >= 0)
        {
            const auto& material = m_mesh.model().materials[primitive.material];

            engine.setDoubleSided(material.doubleSided);

            if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
            {
                engine.bindTexture(0, m_mesh.texture(material.pbrMetallicRoughness.baseColorTexture.index));
                program.setInt("u_baseColorTexture", 0);
                program.setVec4("u_baseColorFactor",
                                glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data()));
            }

            if (material.normalTexture.index >= 0)
            {
                engine.bindTexture(1, m_mesh.texture(material.normalTexture.index));
                program.setInt("u_normalMap", 1);
                program.setFloat("u_normalScale", static_cast<float>(material.normalTexture.scale));
            }
        }
        else
        {
            engine.setDoubleSided(false);
        }

        assert(primitive.indices >= 0); // TODO handle non indexed primitives

        const tinygltf::Accessor& indexAccessor = m_mesh.model().accessors[primitive.indices];

        const GLuint bufferId = m_mesh.buffer(indexAccessor.bufferView);
        vertexArray.bindElementArrayBuffer(bufferId);

        glDrawElements(primitive.mode, static_cast<GLsizei>(indexAccessor.count), indexAccessor.componentType,
                       bufferOffset(indexAccessor.byteOffset));
        // CheckErrors("draw elements");
    }
}

auto MeshRenderer::renderNode(Engine& engine, const int nodeIndex, glm::mat4 transform) -> void
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
            transform *= glm::mat4_cast(glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]));

        if (tr.scale.has_value())
            transform = glm::scale(transform, *tr.scale);
        else if (!node.scale.empty())
            transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
    }

    transform = glm::scale(transform, m_scaleMultiplier[nodeIndex]);

    if (node.mesh > -1)
        renderMesh(engine, node.mesh, transform);
    for (const auto childIndex : node.children)
        renderNode(engine, childIndex, transform);
}

void MeshRenderer::onRender(Engine& engine)
{
    if (!displayed())
        return;
    setPolygoneMode(engine, m_polygonMode);
    for (const auto nodeIndex : m_mesh.model().scenes[m_mesh.model().defaultScene].nodes)
        renderNode(engine, nodeIndex, object().transform().trs());
}
