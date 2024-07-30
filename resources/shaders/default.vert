#version 410

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texcoord;

layout (location = 0) out vec3 FragPos;
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec2 Texcoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;

void main()
{
    vec4 position = projection * view * transform * vec4(in_position, 1.0);
    FragPos = vec3(position);
    gl_Position = position;

    Normal = mat3(transpose(inverse(transform))) * in_normal; // TODO pass normal matrix as argument
    Texcoord = in_texcoord;
}
