#version 410

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texcoord;

layout (location = 0) out vec4 fragColor;

uniform sampler2D texture0;
uniform vec3 camera;
uniform vec4 texColor;

void main() {
    float specular = dot(normalize(normal), normalize(camera)) * 0.4;
    float ambient = 0.4;

    vec4 flatColor = texColor * texture(texture0, texcoord);
    fragColor = flatColor;// * ambient + specular;
}
