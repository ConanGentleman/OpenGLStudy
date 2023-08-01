#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
//���Ⱦ���״ͶӰͼ������۵����������ÿ����һ����
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
//����Ƭ����ɫ���У�����Ϊ�������ÿ��������ɫ�����������ڽ��Ⱦ���״ͶӰͼ������۵����������ÿ����һ����
//Ϊ��ʵ����һ�㣬�����Ȼ�ȡƬ�εĲ���������������Ǵ�������ľֲ�������в�ֵ�õ��ģ�
//Ȼ��ʹ�ô˷���������һЩ����ѧħ���ԵȾ���״ͶӰͼ���в�������ͬ������ͼ����һ��������ֱ�ӽ�����洢��������ÿ�����Ƭ����
void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}