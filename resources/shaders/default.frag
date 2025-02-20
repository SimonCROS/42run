#version 410

#define PI 3.1415926538

layout (location = 0) in vec3 v_FragPos;
#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
layout (location = 1) in mat3 v_TBN;
#else
layout (location = 1) in vec3 v_Normal;
#endif
#endif
#if defined HAS_VEC3_COLORS
layout (location = 2) in vec3 v_color0;
#elif defined HAS_VEC4_COLORS
layout (location = 2) in vec4 v_color0;
#endif
layout (location = 4) in vec2 v_TexCoord;

layout (location = 0) out vec4 FragColor;

#ifdef HAS_BASECOLORMAP
uniform sampler2D u_baseColorTexture;
#endif
#ifdef HAS_METALROUGHNESSMAP
uniform sampler2D u_metallicRoughnessMap;
#endif
#ifdef HAS_NORMALMAP
uniform sampler2D u_normalMap;
#endif
#ifdef HAS_EMISSIVEMAP
uniform sampler2D u_emissiveMap;
#endif
uniform vec4 u_baseColorFactor;
uniform float u_metallicFactor;
uniform float u_roughnessFactor;
uniform float u_normalScale;
uniform vec3 u_emissiveFactor;

uniform vec3 u_cameraPosition;
uniform vec3 u_sunDirection;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

const float c_MinRoughness = 0.04;
const float c_MinMetallic = 0.04;
const float c_Gamma = 2.2;
const float c_GammaInverse = 1 / c_Gamma;
const uint c_Shininess = 5;

// Find the normal for this fragment, pulling either from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal()
{
    // Retrieve the tangent space matrix
#ifndef HAS_TANGENTS
    // Equation explained here https://stackoverflow.com/a/5257471/11448549

    // derivations of the fragment position
    vec3 D = dFdx(v_FragPos);
    vec3 E = dFdy(v_FragPos);
    // derivations of the texture coordinate
    vec2 F = dFdx(v_TexCoord); // TODO if multiple texcoords, take normal texture coords
    vec2 G = dFdy(v_TexCoord);
    // tangent vector and binormal vector
    vec3 T = G.t * D - F.t * E;
    vec3 U = F.s * E - G.s * D;

#ifdef HAS_NORMALS
    vec3 N = normalize(v_Normal);
#else
    vec3 N = cross(D, E);
#endif

    mat3 tbn = mat3(T, U, N);
#else // HAS_TANGENTS
    mat3 tbn = v_TBN;
#endif

#ifdef HAS_NORMALMAP
    vec3 n = texture(u_normalMap, v_TexCoord).rgb;
    n = normalize(n * 2.0 - 1.0); // make it [-1, 1]
    n *= vec3(u_normalScale, u_normalScale, 1.0);
    n *= tbn;
#else
    // The tbn matrix is linearly interpolated, so we need to re-normalize
    vec3 n = normalize(tbn[2].xyz);
#endif

    // // reverse backface normals
    // n *= (2.0 * float(gl_FrontFacing) - 1.0);

    return n;
}

float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 calcPBRDirectionalLight(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness, vec3 lightColor)
{
    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Light Direction and Radiance
    vec3 H = normalize(V + L);
    vec3 radiance = lightColor;            // No attenuation for directional light

    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // Avoid division by zero
    vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    return (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}

void main()
{
    // The albedo may be defined from a base texture or a flat color
    vec4 baseColor = u_baseColorFactor;
#ifdef HAS_BASECOLORMAP
    baseColor *= texture(u_baseColorTexture, v_TexCoord);
#endif
#if defined HAS_VEC3_COLORS
    baseColor *= vec4(v_color0, 1.0f);
#elif defined HAS_VEC4_COLORS
    baseColor *= v_color0;
#endif

    if(baseColor.a < 0.1)
        discard;

    float roughness = u_roughnessFactor;
    float metallic = u_metallicFactor;
#ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    vec4 mrSample = texture(u_metallicRoughnessMap, v_TexCoord); // TODO multiple texcoord possible
    roughness = mrSample.g * roughness;
    metallic = mrSample.b * metallic;
#endif
    roughness = clamp(roughness, c_MinRoughness, 1.0);
    metallic = clamp(metallic, c_MinMetallic, 1.0);

    vec3 ambient = vec3(0.2);
    vec3 normal = getNormal();
    vec3 viewDir = normalize(u_cameraPosition - v_FragPos);

    vec3 result = ambient * baseColor.rgb;
    // --- IN LOOP
    DirectionalLight sunLight;
    sunLight.direction = vec3(4, -6, 8);
    sunLight.color = vec3(10.0, 9.8, 9.0);

    vec3 lightContribution = calcPBRDirectionalLight(normal, viewDir, normalize(-sunLight.direction), baseColor.rgb, metallic, roughness, sunLight.color);
    result += lightContribution;
    // --- NOT IN LOOP

    // Emissive
#ifdef HAS_EMISSIVEMAP
    vec3 emissive = texture(u_emissiveMap, v_TexCoord).rgb;
#else
    vec3 emissive = baseColor.rgb;
#endif
    emissive *= u_emissiveFactor;
    result += emissive;

    result = pow(result, vec3(c_GammaInverse));
    FragColor = vec4(result, baseColor.a);
}
