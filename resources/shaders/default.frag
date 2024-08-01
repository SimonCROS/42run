#version 410

layout (location = 0) in vec3 v_FragPos;
#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
layout (location = 1) in mat3 v_TBN;
#else
layout (location = 1) in vec3 v_Normal;
#endif
#endif
layout (location = 4) in vec2 v_TexCoord;

layout (location = 0) out vec4 FragColor;

#ifdef HAS_BASECOLORMAP
uniform sampler2D baseColorMap;
#endif
#ifdef HAS_METALROUGHNESSMAP
uniform sampler2D metallicRoughnessMap;
#endif
#ifdef HAS_NORMALMAP
uniform sampler2D normalMap;
#endif
uniform float metallicFactor;
uniform float roughnessFactor;
uniform vec4 color;

uniform vec3 viewPos;
uniform vec3 lightPos;

const float c_MinRoughness = 0.04;
const float c_MinMetallic = 0.04;
const float c_Gamma = 2.2;
const float c_GammaInverse = 1 / c_Gamma;

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
    vec3 n = texture(normalMap, v_TexCoord).rgb;
    n = normalize(n * 2.0 - 1.0); // make it [-1, 1]
    n = n * tbn;
#else
    // The tbn matrix is linearly interpolated, so we need to re-normalize
    vec3 n = normalize(tbn[2].xyz);
#endif

    // // reverse backface normals
    // n *= (2.0 * float(gl_FrontFacing) - 1.0);

    return n;
}

void main() {
    // Metallic and Roughness material properties are packed together
    // In glTF, these factors can be specified by fixed scalar values
    // or from a metallic-roughness map
    float perceptualRoughness = roughnessFactor;
    float metallic = metallicFactor;
#ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    vec4 mrSample = texture(metallicRoughnessMap, v_TexCoord); // TODO multiple texcoord possible
    perceptualRoughness = mrSample.g * perceptualRoughness;
    metallic = mrSample.b * metallic;
#endif
    perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);
    metallic = clamp(metallic, c_MinMetallic, 1.0);

    // The albedo may be defined from a base texture or a flat color
#ifdef HAS_BASECOLORMAP
    vec4 baseColor = texture(baseColorMap, v_TexCoord) * color;
#else
    vec4 baseColor = color;
#endif

    vec3 lightColor = vec3(1);

    // ambient
    vec3 ambient = lightColor * 0.1;

    // diffuse
    vec3 norm = getNormal();
    vec3 lightDir = normalize(lightPos - v_FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * perceptualRoughness);

    // specular
    vec3 viewDir = normalize(viewPos - v_FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5);
    vec3 specular = lightColor * (spec * metallic);

    vec3 diffuseColor = baseColor.xyz * (ambient + diffuse);
    vec3 specColor = baseColor.xyz * specular;

    vec3 result = diffuseColor + specColor;
    result = pow(result, vec3(c_GammaInverse));
    FragColor = vec4(result, 1.0);
}
