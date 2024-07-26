#version 410

layout (location = 0) in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;

uniform sampler2D Texture;

void main() {
    FragColor = texture(Texture, TexCoord);
}
