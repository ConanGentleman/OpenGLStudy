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
    if(bloom)//把hdr和泛光纹理混合
        hdrColor += bloomColor; // additive blending
    // 色调映射（从大范围映射到0.0 到1.0
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    //伽马校正  
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}