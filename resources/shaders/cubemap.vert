#version 410

layout (location = 0) in vec3 a_position;

layout (location = 0) out vec4 v_fragPos;

uniform mat4 u_projectionView;

void main()
{
    v_fragPos = u_projectionView * vec4(a_position, 1.0);
    gl_Position = v_fragPos;
}
