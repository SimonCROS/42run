#version 410

layout (location = 0) in vec3 normal;
layout (location = 1) in vec2 texcoord;

layout (location = 0) out vec4 fragColor;

uniform sampler2D Texture;

void main() {
    fragColor = texture(Texture, texcoord);
}
