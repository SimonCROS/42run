#version 410

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;

layout (location = 0) out vec3 normal;
layout (location = 1) out vec2 texcoord;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(in_position, 1.0);
    normal = in_normal;
    texcoord = in_texcoord;
}
