#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float near_plane;
uniform float far_plane;

//��ʹ��͸��ͶӰ����ʱ����ʹ��

//͸��ͶӰ���ڹ�Դ��˵������������⣬�������Լ���λ�õġ�
//͸��ͶӰ��˸��������ڵ��Դ�;۹���ϣ�������ͶӰ�������ڶ�����ϡ�
//��һ��ϸ΢����ǣ�͸��ͶӰ���󣬽���Ȼ����Ӿ���������õ�һ������ȫ�׵Ľ����
//�����������Ϊ͸��ͶӰ�£���ȱ���˷����Ե����ֵ�����Ĵ�����ɱ淶Χ��λ�ڽ�ƽ�渽����
//Ϊ�˿�����ʹ������ͶӰһ�����ʵع۲����ֵ��������Ƚ����������ֵת��Ϊ���Եģ�
//����������Ȳ��Խ̳����Ѿ����۹���������

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // ת����NDC������
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{             
    float depthValue = texture(depthMap, TexCoords).r;
    // FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}