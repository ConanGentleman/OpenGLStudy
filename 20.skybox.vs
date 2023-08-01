#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;//将输入的位置向量作为输出给片段着色器的纹理坐标。片段着色器会将它作为输入来采样samplerCube
    vec4 pos = projection * view * vec4(aPos.x,aPos.y,aPos.z, 1.0);
    gl_Position = pos.xyww;//将输出位置的z分量等于它的w分量，让z分量永远等于1.0
}  