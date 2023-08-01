#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;//每个实例的偏移量

out vec3 fColor;

void main()
{
    fColor = aColor;
    vec2 pos = aPos * (gl_InstanceID / 100.0);//每个实例有对应的gl_InstanceID，从0开始
    gl_Position = vec4(pos + aOffset, 0, 1.0);
}