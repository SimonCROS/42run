#version 410

layout (location = 0) in vec3 a_position;

layout (location = 0) out vec3 v_texCoords;

uniform mat4 u_projectionView;

void main()
{
    v_texCoords = a_position;

    vec4 pos = u_projectionView * vec4(a_position, 1.0);
    gl_Position = pos.xyww;
}
