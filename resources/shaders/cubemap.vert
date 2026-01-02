#version 410

layout (location = 0) in vec3 a_position;

layout (location = 0) out vec3 v_fragPos;

uniform mat4 u_projectionView;

void main()
{
    v_fragPos = a_position;
    gl_Position = u_projectionView * vec4(a_position, 1.0);
}
