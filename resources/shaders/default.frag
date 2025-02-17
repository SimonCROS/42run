#version 410

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

uniform vec3 viewPos;
uniform vec3 lightPos;

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
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

vec3 CalcPointLight(PointLight light, float perceptualRoughness, float metallic, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    // diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), c_Shininess);

    // Apply colors and effects
    vec3 diffuse = light.color * (diff * perceptualRoughness);
    vec3 specular = light.color * (spec * metallic);

    // Light attenuation
    float dist = length(lightPos - v_FragPos);
    float attenuation = light.constant / (1.0 + (light.linear * dist) + (light.quadratic * dist * dist));

    diffuse *= attenuation;
    specular *= attenuation;

    return diffuse + specular;
}

void main() {
    // The albedo may be defined from a base texture or a flat color
    vec4 baseColor = vec4(1, 1, 1, 1);
#ifdef HAS_BASECOLORMAP
    baseColor *= texture(u_baseColorTexture, v_TexCoord) * u_baseColorFactor;
#endif
#if defined HAS_VEC3_COLORS
    baseColor *= vec4(v_color0, 1.0f);
#elif defined HAS_VEC4_COLORS
    baseColor *= v_color0;
#endif

    if(baseColor.a < 0.1)
        discard;

    float perceptualRoughness = u_roughnessFactor;
    float metallic = u_metallicFactor;
#ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    vec4 mrSample = texture(u_metallicRoughnessMap, v_TexCoord); // TODO multiple texcoord possible
    perceptualRoughness = mrSample.g * perceptualRoughness;
    metallic = mrSample.b * metallic;
#endif
    perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);
    metallic = clamp(metallic, c_MinMetallic, 1.0);

    vec3 ambient = vec3(0.1);
    vec3 normal = getNormal();
    vec3 viewDir = normalize(viewPos - v_FragPos);

    vec3 result = ambient * baseColor.rgb;
    // --- IN LOOP
    vec3 light = CalcPointLight(PointLight(lightPos, vec3(1), 1, 0, 0), perceptualRoughness, metallic, normal, v_FragPos, viewDir);
    result += light * baseColor.rgb;
     light = CalcPointLight(PointLight(vec3(40, 42, 10), vec3(1), 1, 0, 0), perceptualRoughness, metallic, normal, v_FragPos, viewDir);
     result += light * baseColor.rgb;
     light = CalcPointLight(PointLight(vec3(10, 23, -5), vec3(1), 1, 0, 0), perceptualRoughness, metallic, normal, v_FragPos, viewDir);
     result += light * baseColor.rgb;
     light = CalcPointLight(PointLight(vec3(-10, 12, -18), vec3(1), 1, 0, 0), perceptualRoughness, metallic, normal, v_FragPos, viewDir);
     result += light * baseColor.rgb;
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
    FragColor = vec4(normal, 1);
}
