#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{    
    // 将片段位置向量存储在第一个gbuffer纹理中
    gPosition = FragPos;
    // 同时将每个片段的法线保存到gbuffer中
    gNormal = normalize(Normal);
    // 同时将每个片段的漫反射保存到gbuffer中
    gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    // 在gAlbedoSpec的alpha中存储高光强度
    gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
}