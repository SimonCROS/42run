#version 410

layout (location = 0) in vec3 a_position;
#ifdef HAS_NORMALS
layout (location = 1) in vec3 a_normal;
#endif
#if defined HAS_VEC3_COLORS
layout (location = 2) in vec3 a_color0;
#elif defined HAS_VEC4_COLORS
layout (location = 2) in vec4 a_color0;
#endif
#ifdef HAS_TEXCOORDS_0
layout (location = 3) in vec2 a_texCoords0;
#endif
#ifdef HAS_TEXCOORDS_1
layout (location = 4) in vec2 a_texCoords1;
#endif
#ifdef HAS_TANGENTS
layout (location = 5) in vec3 a_tangent;
#endif
#ifdef HAS_SKIN
layout (location = 6) in vec4 a_joint;
layout (location = 7) in vec4 a_weight;
#endif

layout (location = 0) out vec3 v_FragPos;
#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
layout (location = 1) out mat3 v_TBN;
#else
layout (location = 1) out vec3 v_Normal;
#endif
#endif
#if defined HAS_VEC3_COLORS
layout (location = 4) out vec3 v_color0;
#elif defined HAS_VEC4_COLORS
layout (location = 4) out vec4 v_color0;
#endif
#if defined HAS_TEXCOORDS_1
layout (location = 5) out vec2 v_texCoords[2];
#elif defined HAS_TEXCOORDS_0
layout (location = 5) out vec2 v_texCoords[1];
#endif

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

    v_FragPos = vec3(
        u_transform *
#ifdef HAS_SKIN
        skinMatrix *
#endif
        vec4(a_position, 1.0)
    );

#ifdef HAS_NORMALS
    mat3 rsMatrix = mat3(u_transform);
#ifdef HAS_SKIN
    rsMatrix *= mat3(skinMatrix);
#endif

    mat3 normalMatrix = transpose(inverse(rsMatrix)); // TODO pass normal matrix as argument
    vec3 N = normalize(normalMatrix * a_normal);
#ifdef HAS_TANGENTS
    vec3 T = normalize(normalMatrix * a_tangent);
    // re-orthogonalize T with respect to N (Gram-Schmidt process)
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);
    v_TBN = TBN;
//    v_TangentLightPos = TBN * u_lightPos;
//    v_TangentViewPos  = TBN * u_viewPos;
//    v_TangentFragPos  = TBN * v_FragPos;
#else
    v_Normal = N;
#endif
#endif

#ifdef HAS_TEXCOORDS_0
    v_texCoords[0] = a_texCoords0;
#endif
#ifdef HAS_TEXCOORDS_1
    v_texCoords[1] = a_texCoords1;
#endif
}
