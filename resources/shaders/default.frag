#version 410

layout (location = 0) in vec3 Color;
layout (location = 1) in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;

uniform sampler2D Texture;

void main() {
    FragColor = texture(Texture, TexCoord) * vec4(Color, 1.0);
}
