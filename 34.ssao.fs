#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

// ������Ļ�ߴ����������С��ƽ�̣�Tile����������
//ע������������һ��noiseScale�ı�����������Ҫ����������ƽ��(Tile)����Ļ�ϣ�
//��������TexCoords��ȡֵ��0.0��1.0֮�䣬texNoise��������ƽ�̡�
//�������ǽ�ͨ����Ļ�ֱ��ʳ������������С�ķ�ʽ����TexCoords�����Ŵ�С������֮����ȡ�������������ʱ��ʹ�á�
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); 

uniform mat4 projection;
//������Ҫ���ű��淨�߷������ɴ�������������������������̵̳Ŀ�ʼ���ܵ�������������Ҫ�����γɰ����ε�������
//�����ڶ�ÿ�����淨�߷������ɲ������ķǳ����ѣ�Ҳ����ʵ�ʣ�

//������ǽ������߿ռ�(Tangent Space)�����ɲ������ģ���������ָ����z����
void main()
{
    // ��ȡSSAO�㷨������
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);//����һ��rgb�ĳ�xyz�������û̫��仯��
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    // ����TBN���任����:�����߿ռ䵽�۲�ռ�
    //ͨ��ʹ��һ������Gramm-Schmidt����(Gramm-Schmidt Process)�Ĺ��̣����Ǵ�����һ��������(Orthogonal Basis)��
    //ÿһ�����������randomVec��ֵ��΢��б��ע����Ϊ����ʹ����һ�������������������������
    //����û��Ҫ��һ��ǡ�����ż���������TBN����Ҳ���ǲ���Ҫ�𶥵�����(��˫��)������
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // �����������Ĳ������ڵ�����
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // �õ�����λ��
        //����ÿһ�ε����������ȱ任�����������۲�ռ䡣
        //֮�����ǻ�ӹ۲�ռ����ƫ���������۲�ռ�Ƭ��λ���ϣ�
        //�������radius����ƫ������������(�����)SSAO����Чȡ���뾶��
        vec3 samplePos = TBN * samples[i]; // �����߿ռ䵽�۲�ռ�
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        //ͶӰ����λ�ã������������������Ļ�ϵ�λ��/����
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // �ӹ۲�ռ�ת�����ü��ռ�
        offset.xyz /= offset.w; // ͸�ӻ���
        offset.xyz = offset.xyz * 0.5 + 0.5; // �任��0.0 - 1.0��ֵ��
        
        //�õ��������
        //����ʹ��offset������x��y�������������������Ӷ���ȡ����λ�ôӹ۲����ӽǵ����ֵ(��һ�������ڱεĿɼ�Ƭ��)��
        float sampleDepth = texture(gPosition, offset.xy).z; //�õ��������������ֵ
        
        // ��Χ���ͼ���
        //���ǽ�������������ĵ�ǰ���ֵ�Ƿ���ڴ洢�����ֵ������ǵģ���ӵ����յĹ��������ϡ�

        //�����һ�����������Ե��Ƭ��ʱ�������ῼ�ǲ��Ա���֮�µı�������ֵ��
        //��Щֵ����(����ȷ��)Ӱ���ڱ����ӡ����ǿ���ͨ������һ����Χ���Ӷ����������⣬������ͼ��ʾ(John Chapman�ķ���)��
        //��������һ����Χ���ԴӶ���֤����ֻ���������ֵ��ȡ���뾶��ʱӰ���ڱ�����
        //��������ʹ����GLSL��smoothstep���������ǳ��⻬���ڵ�һ�͵ڶ���������Χ�ڲ�ֵ�˵����������������Ȳ��������ȡֵ��radius֮�䣬���ǵ�ֵ����⻬�ظ�������������߲�ֵ��0.0��1.0֮�䣺
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        //�������ʹ��һ�������ֵ��radius֮���ͻȻ�Ƴ��ڱι��׵�Ӳ���޷�Χ���(Hard Cut-off Range Check)�����ǽ����ڷ�Χ���Ӧ�õĵط�����һ�����Ե�(���ѿ���)��Ե��(��ߵ����ָо���
        
        //���ǽ�������������ĵ�ǰ���ֵ�Ƿ���ڴ洢�����ֵ������ǵģ���ӵ����յĹ��������ϡ�
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    //���һ����������Ҫ���ڱι��׸��ݺ��ĵĴ�С��׼��������������
    //ע��������1.0��ȥ���ڱ����ӣ��Ա�ֱ��ʹ���ڱ�����ȥ���Ż������շ�����
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = occlusion;
}