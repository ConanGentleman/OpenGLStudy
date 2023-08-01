#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{             
    vec4 texColor = texture(texture1, TexCoords);
    if(texColor.a < 0.1)//͸����С��
        discard;//discard���һ�������ã����ͻᱣ֤Ƭ�β��ᱻ��һ���������ԾͲ��������ɫ����.�������ָ����Ǿ��ܹ���Ƭ����ɫ���м��һ��Ƭ�ε�alphaֵ�Ƿ����ĳ����ֵ������ǵĻ����������Ƭ�Σ��ͺ�����������һ��
    FragColor = texColor;
}