#version 410

layout (location = 0) in vec3 FragPos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 Texcoord;

layout (location = 0) out vec4 FragColor;

uniform sampler2D albedoMap;
uniform sampler2D metallicRoughnessMap;
uniform float metallicFactor;
uniform float roughnessFactor;
uniform vec4 color;

uniform vec3 viewPos;
uniform vec3 lightPos;

const float c_MinRoughness = 0.04;
const float c_MinMetallic = 0.04;

void main() {
    // Metallic and Roughness material properties are packed together
    // In glTF, these factors can be specified by fixed scalar values
    // or from a metallic-roughness map
    float perceptualRoughness = roughnessFactor;
    float metallic = metallicFactor;
// #ifdef HAS_METALROUGHNESSMAP
    // Roughness is stored in the 'g' channel, metallic is stored in the 'b' channel.
    // This layout intentionally reserves the 'r' channel for (optional) occlusion map data
    vec4 mrSample = texture(metallicRoughnessMap, Texcoord); // TODO multiple texcoord possible
    perceptualRoughness = mrSample.g * perceptualRoughness;
    metallic = mrSample.b * metallic;
// #endif
    perceptualRoughness = clamp(perceptualRoughness, c_MinRoughness, 1.0);
    metallic = clamp(metallic, c_MinMetallic, 1.0);

    // The albedo may be defined from a base texture or a flat color
// #ifdef HAS_BASECOLORMAP
    vec4 baseColor = texture(albedoMap, Texcoord) * color;
// #else
    // vec4 baseColor = color;
// #endif

    vec3 lightColor = vec3(1);

    // ambient
    vec3 ambient = lightColor * 0.7;

    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightColor * (diff * perceptualRoughness);

    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5);
    vec3 specular = lightColor * (spec * metallic);

    vec3 diffuseColor = baseColor.xyz * (ambient + diffuse);
    vec3 specColor = baseColor.xyz * specular;

    vec3 result = diffuseColor + specColor;
    FragColor = vec4(result, 1.0);
}
