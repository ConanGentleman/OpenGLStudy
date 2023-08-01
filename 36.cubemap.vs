#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;
//Ҫ���Ⱦ���״ͶӰͼת��Ϊ��������ͼ��������Ҫ��Ⱦһ������λ�������壬�����ڲ����Ⱦ���״ͼͶӰ���������ÿ���棬
//������������������ͼ�������������ͼ����������Ķ�����ɫ��ֻ�ǰ�ԭ����Ⱦ�����壬������ֲ�������Ϊ 3D �����������ݸ�Ƭ����ɫ����
void main()
{
    WorldPos = aPos;  
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}