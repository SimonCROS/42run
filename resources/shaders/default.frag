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
layout (location = 4) in vec3 v_color0;
#elif defined HAS_VEC4_COLORS
layout (location = 4) in vec4 v_color0;
#endif
#if defined HAS_TEXCOORD_1
layout (location = 5) in vec2 v_texCoords[2];
#elif defined HAS_TEXCOORD_0
layout (location = 5) in vec2 v_texCoords[1];
#endif

layout (location = 0) out vec4 FragColor;

#ifdef HAS_BASECOLORMAP
uniform sampler2D u_baseColorTexture;
uniform uint u_baseColorTexCoordIndex;
#endif
#ifdef HAS_METALROUGHNESSMAP
uniform sampler2D u_metallicRoughnessMap;
uniform uint u_metallicRoughnessTexCoordIndex;
#endif
#ifdef HAS_NORMALMAP
uniform sampler2D u_normalMap;
uniform uint u_normalTexCoordIndex;
#endif
#ifdef HAS_EMISSIVEMAP
uniform sampler2D u_emissiveMap;
uniform uint u_emissiveTexCoordIndex;
#endif

uniform vec4 u_baseColorFactor;
uniform float u_metallicFactor;
uniform float u_roughnessFactor;
uniform float u_normalScale;
uniform vec3 u_emissiveFactor;
uniform samplerCube u_cubemap;

uniform vec3 u_cameraPosition;
uniform vec3 u_sunDirection;
uniform vec4 u_fogColor;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

const float c_MinRoughness = 0.04;
const float c_MinMetallic = 0.04;
const float c_Gamma = 2.2;
const float c_GammaInverse = 1 / c_Gamma;
const uint c_Shininess = 5;

mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    // solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame
    float invmax = inversesqrt(max(dot(T,T), dot(B,B)));
    return mat3(T * invmax, B * invmax, N);
}

// Find the normal for this fragment, pulling either from a predefined normal map
// or from the interpolated mesh normal and tangent attributes.
vec3 getNormal()
{
    // Retrieve the tangent space matrix
#ifdef HAS_TANGENTS
    mat3 tbn = v_TBN;
#else
#ifdef HAS_TEXCOORD_0
    vec2 uv = v_texCoords[0];
#else
    vec2 uv = v_FragPos.xy;
#endif
#ifdef HAS_NORMALS
    vec3 N = v_Normal;
#else
    vec3 N = normalize(cross(dFdx(v_FragPos), dFdy(v_FragPos)));
#endif
    mat3 tbn = cotangent_frame(N, v_FragPos, uv);
#endif

#ifdef HAS_NORMALMAP
    vec3 n = texture(u_normalMap, v_texCoords[u_normalTexCoordIndex]).rgb;
    n = n * 2.0 - 1.0; // make it [-1, 1]
    n = normalize(tbn * n);
    n *= vec3(u_normalScale, u_normalScale, 1.0);
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

vec3 calcPBRDirectionalLight(
    vec3 N,             // Normal (world space)
    vec3 V,             // View direction (normalized, world space)
    vec3 L,             // Light direction (normalized, world space, points TO light)
    vec3 albedo,        // Base color (non-metal: diffuse, metal: reflective tint)
    float metallic,     // 0 = dielectric, 1 = metal
    float roughness,    // 0 = smooth, 1 = rough
    vec3 lightColor     // Light radiance (RGB color * intensity)
) {
    // Clamp roughness to prevent division issues
    roughness = clamp(roughness, 0.05, 1.0);

    // Half vector
    vec3 H = normalize(V + L);

    // Fresnel reflectance at normal incidence
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // ----- Cook-Torrance BRDF -----

    // Normal Distribution Function (GGX)
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denomNDF = (NdotH2 * (a2 - 1.0) + 1.0);
    float D = a2 / (PI * denomNDF * denomNDF);

    // Geometry function (Smith, Schlick-GGX)
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float k = (roughness + 1.0);
    k = (k * k) / 8.0;

    float G_V = NdotV / (NdotV * (1.0 - k) + k);
    float G_L = NdotL / (NdotL * (1.0 - k) + k);
    float G = G_V * G_L;

    // Fresnel term (Schlick approximation)
    float HdotV = max(dot(H, V), 0.0);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);

    // Final specular term
    vec3 specular = (D * G * F) / max(4.0 * NdotL * NdotV, 0.001);

    // Diffuse term (Lambert) — no diffuse for metals
    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    // Final light contribution
    vec3 result = (diffuse + specular) * lightColor * NdotL;

    return result;
}

void main()
{
    float dist = length(u_cameraPosition - v_FragPos);
    float fogFactor = max(0, dist - 30) * 0.05;
    fogFactor *= fogFactor;
    if (fogFactor >= 1)
        discard;

    // The albedo may be defined from a base texture or a flat color
    vec4 baseColor = u_baseColorFactor;
#ifdef HAS_BASECOLORMAP
    baseColor *= texture(u_baseColorTexture, v_texCoords[u_baseColorTexCoordIndex]);
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
    vec4 mrSample = texture(u_metallicRoughnessMap, v_texCoords[u_metallicRoughnessTexCoordIndex]);
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
    sunLight.direction = vec3(4, -6, -8);
    sunLight.color = vec3(10.0, 9.8, 9.0);

    vec3 lightContribution = calcPBRDirectionalLight(normal, viewDir, normalize(-sunLight.direction), baseColor.rgb, metallic, roughness, sunLight.color);
    FragColor = vec4(lightContribution, 1.0f);
    result += lightContribution;

    sunLight.direction = vec3(0, -6, 0);
    sunLight.color = vec3(5.0, 4.9, 4.5);
    lightContribution = calcPBRDirectionalLight(normal, viewDir, normalize(-sunLight.direction), baseColor.rgb, metallic, roughness, sunLight.color);
    result += lightContribution;
    // --- NOT IN LOOP

    // Emissive
#ifdef HAS_EMISSIVEMAP
    vec3 emissive = texture(u_emissiveMap, v_texCoords[u_emissiveTexCoordIndex]).rgb;
#else
    vec3 emissive = baseColor.rgb;
#endif
    emissive *= u_emissiveFactor;
    result += emissive;

//    {
//        // Base reflectivity for non-metals (typical dielectric value)
//        float baseReflectivity = 0.04;
//
//        // Interpolate between non-metal and full metal reflectivity
//        float F0 = mix(baseReflectivity, 1.0, metallic);
//
//        // Approximate Fresnel-Schlick as a function of roughness
//        float fresnel = F0 + (1.0 - F0) * pow(1.0 - roughness, 5.0);
//
//        // Adjust reflectance based on roughness (rougher surfaces reflect less)
//        float roughnessEffect = 1.0 - roughness;
//
//        // Combine factors
//        float reflectance = fresnel * roughnessEffect;
//
//        // Ensure the result is in the [0, 1] range
//        result = vec3(clamp(reflectance, 0.0, 1.0), 0, 0);
//    }

    result = pow(result, vec3(c_GammaInverse));
    result = mix(result, vec3(u_fogColor), fogFactor);

    FragColor = vec4(result, baseColor.a);
}
