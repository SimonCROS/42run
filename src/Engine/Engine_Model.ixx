//
// Created by Simon Cros on 26/01/2025.
//

module;

#include "glad/gl.h"
#include "tiny_gltf.h"

export module Engine:Mesh;
import std;
import Engine.Animation;
import Engine.RenderInfo;
import :Engine;
import OpenGL;

export class Model
{
private:
    std::vector<GLuint> m_textures;
    std::vector<Animation> m_animations; // TODO use a pointer to ensure location never change and faster access
    std::vector<Material> m_materials;
    ModelRenderInfo m_renderInfo;

public:
    static auto Create(Engine & engine, const tinygltf::Model & model) -> Model;

    Model(std::vector<GLuint> && textures, std::vector<Animation> && animations, std::vector<Material> && materials,
          ModelRenderInfo && renderInfo) : m_textures(std::move(textures)),
                                           m_animations(std::move(animations)),
                                           m_materials(std::move(materials)),
                                           m_renderInfo(std::move(renderInfo))
    {}

    [[nodiscard]] auto texture(const size_t index) const -> GLuint { return m_textures[index]; }

    [[nodiscard]] auto animations() const -> const std::vector<Animation> & { return m_animations; }

    [[nodiscard]] auto materials() const -> const std::vector<Material> & { return m_materials; }

    [[nodiscard]] auto renderInfo() const -> const ModelRenderInfo & { return m_renderInfo; }

    [[nodiscard]] auto prepareShaderPrograms(ShaderProgram & builder) const -> std::expected<void, std::string>
    {
        const auto & renderInfo = m_renderInfo;
        for (int i = 0; i < renderInfo.meshesCount; ++i)
        {
            const auto & mesh = renderInfo.meshes[i];
            for (int j = 0; j < mesh.primitivesCount; ++j)
            {
                const auto & primitive = mesh.primitives[j];

                auto e_success = builder.enableVariant(primitive.shaderFlags);
                if (!e_success)
                    return std::unexpected(std::move(e_success).error());
            }
        }

        return {};
    }
};
