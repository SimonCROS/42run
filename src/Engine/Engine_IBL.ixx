//
// Created by Simon Cros on 29/06/2026.
//

module;

#include <cstdio>
#include "glad/gl.h"

export module Engine:IBL;
import OpenGL.Cubemap;
import OpenGL.Texture2D;

constexpr GLuint cubemapSize = 512;

export class IBL
{
private:
    OpenGL::Cubemap m_irradianceMap;
    OpenGL::Cubemap m_prefilterMap;
    OpenGL::Texture2D m_brdfTexture;

    IBL(OpenGL::Cubemap && irradianceMap, OpenGL::Cubemap && prefilterMap,
        OpenGL::Texture2D && brdf) : m_irradianceMap(std::move(irradianceMap)),
                                     m_prefilterMap(std::move(prefilterMap)),
                                     m_brdfTexture(std::move(brdf)) {}

    static auto generateSkybox(OpenGL::Cubemap & output, const OpenGL::Texture2D & input,
                        ShaderProgram & program) -> std::expected<void, std::string>
    {
        glUseProgram(program.id());
        input.bind(GL_TEXTURE0);
        program.setInt("u_equirectangularMap", 0);

        TRY(output.fromShader(program, 0));
        output.generateMipmap();
        return {};
    }

    static auto generateIrradianceMap(OpenGL::Cubemap & output, const OpenGL::Cubemap & input,
                               ShaderProgram & program) -> std::expected<void, std::string>
    {
        glUseProgram(program.id());
        input.bind(GL_TEXTURE0);
        program.setInt("u_cubemap", 0);

        TRY(output.fromShader(program, 0));
        return {};
    }

    static auto generatePrefilterMap(OpenGL::Cubemap & output, const OpenGL::Cubemap & input,
                              ShaderProgram & program) -> std::expected<void, std::string>
    {
        constexpr int maxLevel = 4; // TODO get from output

        glUseProgram(program.id());
        input.bind(GL_TEXTURE0);
        program.setInt("u_cubemap", 0);

        for (int i = 0; i <= maxLevel; ++i)
        {
            program.setFloat("u_roughness", static_cast<float>(i) / maxLevel);
            TRY(output.fromShader(program, i));
        }

        return {};
    }

public:
    IBL() = delete;

    IBL(const IBL &) = delete;

    IBL(IBL &&) = default;

    auto operator=(const IBL &) -> IBL & = delete;

    auto operator=(IBL &&) -> IBL & = default;

    static auto Create() -> std::expected<IBL, std::string>
    {
        TRY_V(auto, irradianceMap, OpenGL::createCubemap(*stateCache.get(), {
                  .internalFormat = GL_RGB32F,
                  .size = cubemapSize,
                  .maxLevel = 4,
                  .debugLabel = "Irradiance"
                  }));

        TRY_V(auto, prefilterMap, OpenGL::createCubemap(*stateCache.get(), {
                  .internalFormat = GL_RGB32F,
                  .size = cubemapSize,
                  .minFilter = GL_LINEAR_MIPMAP_LINEAR,
                  .maxLevel = 4,
                  .debugLabel = "Prefilter"
                  }));

        TRY_V(auto, brdfTexture, OpenGL::Texture2D::builder(stateCache.get())
              .internalFormat(GL_RG16F)
              .size(cubemapSize, cubemapSize)
              // .debugLabel("BRDF")
              .build());

        return {
            std::move(irradianceMap), std::move(prefilterMap), std::move(brdfTexture)
        };
    }

    auto load() -> std::expected<void, std::string>
    {
        const bool irradianceLoaded = m_irradianceMap.fromCache(".cache/irradiance.cubemap", GL_RGB, GL_FLOAT);
        const bool prefilterLoaded = m_prefilterMap.fromCache(".cache/prefilter.cubemap", GL_RGB, GL_FLOAT);
        if (!irradianceLoaded || !prefilterLoaded)
        {
            TRY_V(auto, hdrImage, Image::Create(RESOURCE_PATH"textures/skybox/san_giuseppe_bridge_1k.hdr"));
            TRY_V(auto, hdrTexture, OpenGL::Texture2D::builder(stateCache.get())
                  .internalFormat(GL_RGB32F)
                  .size(hdrImage.width(), hdrImage.height())
                  // .debugLabel("Equirectangular Skybox")
                  .build());
            TRY_V(auto, skybox, OpenGL::createCubemap(*stateCache.get(), {
                      .internalFormat = GL_RGB32F,
                      .size = cubemapSize,
                      .debugLabel = "Skybox"
                      }));

            hdrTexture.fromRaw(hdrImage.glFormat(), hdrImage.glType(), hdrImage.data());
            TRY(generateSkybox(
                skybox,
                hdrTexture,
                engine.getShaderManager().getProgram(eqProgramIdx)));

            if (!irradianceLoaded)
            {
                TRY(generateIrradianceMap(
                    m_irradianceMap,
                    skybox,
                    engine.getShaderManager().getProgram(irradianceProgramIdx)));
                TRY_LOG(m_irradianceMap.saveCache(".cache/irradiance.cubemap", GL_RGB, GL_FLOAT));
            }

            if (!prefilterLoaded)
            {
                TRY(generatePrefilterMap(
                    m_prefilterMap,
                    skybox,
                    engine.getShaderManager().getProgram(prefilterProgramIdx)));
                TRY_LOG(m_prefilterMap.saveCache(".cache/prefilter.cubemap", GL_RGB, GL_FLOAT));
            }
        }

        if (!m_brdfTexture.fromCache(".cache/brdf.texture2d", GL_RG, GL_HALF_FLOAT))
        {
            TRY(m_brdfTexture.fromShader(engine.getShaderManager().getProgram(brdfProgramIdx)));
            TRY_LOG(m_brdfTexture.saveCache(".cache/brdf.texture2d", GL_RG, GL_HALF_FLOAT));
        }
    }
};
