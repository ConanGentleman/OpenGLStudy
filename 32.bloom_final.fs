#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if(bloom)//��hdr�ͷ���������
        hdrColor += bloomColor; // additive blending
    // ɫ��ӳ�䣨�Ӵ�Χӳ�䵽0.0 ��1.0
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    //٤��У��  
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}