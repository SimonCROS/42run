#version 410

layout (location = 0) in vec2 v_texCoords;

layout (location = 0) out vec4 f_color;

uniform sampler2D u_hdrBuffer;
uniform bool u_hdr;
uniform float u_exposure;

const float c_gamma = 2.2;
const float c_gammaInverse = 1 / c_gamma;

void main()
{
    vec3 hdrColor = texture(u_hdrBuffer, v_texCoords).rgb;
    vec3 result;

    if(u_hdr)
        result = vec3(1.0) - exp(-hdrColor * u_exposure);
    else
        result = pow(result, vec3(c_gammaInverse));

    result = pow(result, vec3(c_gammaInverse));
    f_color = vec4(result, 1.0);
}
