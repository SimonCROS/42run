#version 410

layout (location = 0) in vec3 a_Position;
#ifdef HAS_NORMALS
layout (location = 1) in vec3 a_Normal;
#endif
#ifdef HAS_TANGENTS
layout (location = 2) in vec3 a_Tangent;
#endif
layout (location = 3) in vec2 a_TexCoord;

layout (location = 0) out vec3 v_FragPos;
#ifdef HAS_NORMALS
#ifdef HAS_TANGENTS
layout (location = 1) out mat3 v_TBN;
#else
layout (location = 1) out vec3 v_Normal;
#endif
#endif
layout (location = 4) out vec2 v_TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;
uniform vec3 viewPos;
uniform vec3 lightPos;

void main()
{
    gl_Position = projection * view * transform * vec4(a_Position, 1.0);
    v_FragPos = vec3(transform * vec4(a_Position, 1.0));

#ifdef HAS_NORMALS
    mat3 normalMatrix = transpose(inverse(mat3(transform))); // TODO pass normal matrix as argument
    vec3 N = normalize(normalMatrix * a_Normal);
#ifdef HAS_TANGENTS
    vec3 T = normalize(normalMatrix * a_Tangent);
    // re-orthogonalize T with respect to N (Gram-Schmidt process)
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    mat3 TBN = transpose(mat3(T, B, N));
    v_TBN = TBN;
//    v_TangentLightPos = TBN * lightPos;
//    v_TangentViewPos  = TBN * viewPos;
//    v_TangentFragPos  = TBN * v_FragPos;
#else
    v_Normal = N;
#endif
#endif

    v_TexCoord = a_TexCoord;
}
