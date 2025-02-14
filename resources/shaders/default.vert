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
layout (location = 3) in vec2 a_texCoord0;
#ifdef HAS_TANGENTS
layout (location = 4) in vec3 a_tangent;
#endif

layout (location = 0) out vec3 v_FragPos;
#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
layout (location = 1) out mat3 v_TBN;
#else
layout (location = 1) out vec3 v_Normal;
#endif
#endif
layout (location = 4) out vec2 v_TexCoord;

uniform mat4 u_projectionView;
uniform mat4 u_transform;
uniform vec3 u_viewPos;
uniform vec3 u_lightPos;

void main()
{
    gl_Position = u_projectionView * u_transform * vec4(a_position, 1.0);
    v_FragPos = vec3(u_transform * vec4(a_position, 1.0));

#ifdef HAS_NORMALS
    mat3 normalMatrix = transpose(inverse(mat3(u_transform))); // TODO pass normal matrix as argument
    vec3 N = normalize(normalMatrix * a_normal);
#ifdef HAS_TANGENTS
    vec3 T = normalize(normalMatrix * a_tangent);
    // re-orthogonalize T with respect to N (Gram-Schmidt process)
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));
    v_TBN = TBN;
//    v_TangentLightPos = TBN * u_lightPos;
//    v_TangentViewPos  = TBN * u_viewPos;
//    v_TangentFragPos  = TBN * v_FragPos;
#else
    v_Normal = N;
#endif
#endif

    v_TexCoord = a_texCoord0;
}
