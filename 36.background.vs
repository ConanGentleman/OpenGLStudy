#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;
//天空盒
void main()
{
    WorldPos = aPos;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);
	//注意这里的小技巧 xyww 可以确保渲染的立方体片段的深度值总是 1.0，即最大深度，如立方体贴图教程中所述。注意我们需要将深度比较函数更改为 GL_LEQUAL：
	gl_Position = clipPos.xyww;
}