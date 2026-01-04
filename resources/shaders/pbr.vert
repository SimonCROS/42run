#version 410

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
// layout (location = 2) in vec3 a_color0;
layout (location = 2) in vec4 a_color0;
layout (location = 3) in vec2 a_texCoords0;
layout (location = 4) in vec2 a_texCoords1;
layout (location = 5) in vec4 a_tangent;
layout (location = 6) in vec4 a_joint;
layout (location = 7) in vec4 a_weight;

layout (location = 0) out vec3 v_position;
layout (location = 1) out vec3 v_normal;
layout (location = 2) out vec4 v_tangent;
// layout (location = 4) out vec3 v_color0;
layout (location = 4) out vec4 v_color0;
layout (location = 5) out vec2 v_texCoords[2];

uniform mat4 u_projectionView;
uniform mat4 u_transform;
uniform vec3 u_viewPos;
uniform vec3 u_lightPos;
#ifdef HAS_SKIN
layout(std140) uniform JointMatrices {
    mat4 u_jointMatrix[MAX_JOINTS];
};
#endif

void main()
{
#ifdef HAS_SKIN
    mat4 skinMatrix =
        a_weight.x * u_jointMatrix[int(a_joint.x)] +
        a_weight.y * u_jointMatrix[int(a_joint.y)] +
        a_weight.z * u_jointMatrix[int(a_joint.z)] +
        a_weight.w * u_jointMatrix[int(a_joint.w)];
#endif

    gl_Position =
        u_projectionView *
        u_transform *
#ifdef HAS_SKIN
        skinMatrix *
#endif
        vec4(a_position, 1.0);

    v_position = vec3(
        u_transform *
#ifdef HAS_SKIN
        skinMatrix *
#endif
        vec4(a_position, 1.0)
    );

    mat3 transform3 = mat3(u_transform);
#ifdef HAS_SKIN
    transform3 *= mat3(skinMatrix);
#endif

    mat3 normalMatrix = transpose(inverse(transform3)); // TODO pass normal matrix as argument
    v_normal = normalize(normalMatrix * a_normal);

    v_tangent.xyz = normalize(transform3 * a_tangent.xyz);
    v_tangent.w = a_tangent.w;

    v_texCoords[0] = a_texCoords0;
    v_texCoords[1] = a_texCoords1;

    v_color0 = a_color0;
}
