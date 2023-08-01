#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

void main()
{    
    // 将片段位置向量存储在第一个gbuffer纹理中
    gPosition = FragPos;
    // 同时将每个片段的法线保存到gbuffer中
    gNormal = normalize(Normal);
    // 以及每个片段的漫反射颜色
    gAlbedo.rgb = vec3(0.95);
}