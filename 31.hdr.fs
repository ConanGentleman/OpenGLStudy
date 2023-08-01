#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure; //�ع����

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    if(hdr)//ʹ��ɫ��ӳ���㷨
    {
        //Reinhard ɫ��ӳ���㷨ƽ���ؽ���������ֵ��ɢ��LDR��
        // vec3 result = hdrColor / (hdrColor + vec3(1.0));
        
        
        //�ع�ɫ��ӳ��
        //���������һ������Ҫչ����ҹ���棬���ǵ�Ȼ���ڰ���ʹ�õ��ع⣬��ҹ��ʹ�ø��ع⣬
        //�������۵��ڷ�ʽһ������������ع������
        //���ǿ���ȥ���ÿ���ͬʱ�ڰ����ҹ��ͬ�������������Ĺ��ղ�����
        //����ֻ��Ҫ�����ع���������ˡ�
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        // gamma����     
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else
    {
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
}