#version 410

layout (location = 0) in vec3 v_texCoords;

layout (location = 0) out vec4 f_color;

uniform samplerCube u_cubemap;

void main()
{
    f_color = texture(u_cubemap, v_texCoords);
}
