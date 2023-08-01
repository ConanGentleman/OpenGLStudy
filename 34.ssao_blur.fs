#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoInput;

void main() 
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    //�������Ǳ�������Χ��-2.0��2.0֮���SSAO����Ԫ(Texel)����������������ά����ͬ������SSAO����
    //����ͨ��ʹ�÷���vec2����ά�ȵ�textureSize����������Ԫ����ʵ��Сƫ����ÿһ���������ꡣ
    //����ƽ�����õĽ�������һ���򵥵�����Ч��ģ��Ч����
    for (int x = -2; x < 2; ++x) //��ֱˮƽ�����
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, TexCoords + offset).r;
        }
    }
    FragColor = result / (4.0 * 4.0);//����������ά��(4*4)
}  