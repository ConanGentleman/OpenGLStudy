#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{    
    vec3 I = normalize(Position - cameraPos);//观察/摄像机方向向量I，并使用它来计算反射向量R
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);//使用R来从天空盒立方体贴图中采样
}