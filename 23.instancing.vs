#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;//ÿ��ʵ����ƫ����

out vec3 fColor;

void main()
{
    fColor = aColor;
    vec2 pos = aPos * (gl_InstanceID / 100.0);//ÿ��ʵ���ж�Ӧ��gl_InstanceID����0��ʼ
    gl_Position = vec4(pos + aOffset, 0, 1.0);
}