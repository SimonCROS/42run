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
layout (location = 1) out mat3 v_Normal;
#endif
#endif
layout (location = 4) out vec2 v_TexCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 transform;

void main()
{
    vec4 position = projection * view * transform * vec4(a_Position, 1.0);
    v_FragPos = vec3(position);
    gl_Position = position;

#if HAS_NORMALS
    mat3 normalMatrix = mat3(transpose(inverse(transform))); // TODO pass normal matrix as argument
    vec3 normalW = normalize(normalMatrix * a_Normal);
#if HAS_TANGENTS
    vec3 tangentW = normalize(normalMatrix * a_Tangent);
    vec3 bitangentW = cross(normalW, tangentW);
    v_TBN = mat3(tangentW, bitangentW, normalW);
#else
    v_Normal = normalW;
#endif
#endif

    v_TexCoord = a_TexCoord;
}
