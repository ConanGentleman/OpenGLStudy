#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float near_plane;
uniform float far_plane;

//在使用透视投影矩阵时必须使用

//透视投影对于光源来说更合理，不像定向光，它是有自己的位置的。
//透视投影因此更经常用在点光源和聚光灯上，而正交投影经常用在定向光上。
//另一个细微差别是，透视投影矩阵，将深度缓冲视觉化经常会得到一个几乎全白的结果。
//发生这个是因为透视投影下，深度变成了非线性的深度值，它的大多数可辨范围都位于近平面附近。
//为了可以像使用正交投影一样合适地观察深度值，你必须先将非线性深度值转变为线性的，
//如我们在深度测试教程中已经讨论过的那样。

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // 转换到NDC坐标下
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{             
    float depthValue = texture(depthMap, TexCoords).r;
    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}