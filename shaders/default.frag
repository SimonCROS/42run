#version 450

layout (location = 0) in vec3 i_color;

layout (location = 0) out vec3 o_color;

void main() {
    o_color = i_color;
}
