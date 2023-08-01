#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
//���߷ֲ�����
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
//��Ч��VanDerCorpus����
//�ڱ��̳��У����ǽ�ʹ����Ҫ�Բ�����Ԥ�����ӷ��䷽�̵ľ��淴�䲿�֣��ò������������ؿ��巽������������ĵͲ�������
//����һЩ����ļ��ɣ����ǿ�������ɫ�������зǳ���Ч������ Van Der Corput ���У����ǽ���������� Hammersley ���У�����������Ϊ N����������Ϊ i��
//��Ͳ�������
float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
//GLSL �� Hammersley �������Ի�ȡ��СΪ N ���������еĵͲ������� i
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
//��Ҫ�Բ���
//Ҫ��������������������ҪһЩ���������ƫ�Ʋ�����������ʹ�䳯���ض��ֲڶȵľ��沨�귽��
//���ǿ��������۽̳�������ʹ�� NDF������ GGX NDF ��ϵ� Epic Games ���������β��������Ĵ����У�
//�����ض��Ĵֲڶ�����͵Ͳ�������ֵ Xi�����ǻ����һ����������������������Χ����Ԥ����΢����İ�������
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	//�������굽�ѿ�������-�������
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	//���пռ��H����������ռ����������
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
// ----------------------------------------------------------------------------

void main()
{		
    vec3 N = normalize(WorldPos);
    
    //�򻯼���V = R =������ 
    //����ʹ�� Cook-Torrance BRDF �ķ��߷ֲ�����(NDF)���ɲ�����������ɢ��ǿ�ȣ��ú��������ߺ��ӽǷ�����Ϊ���롣
    //���������ھ���������ͼʱ���Ȳ�֪���ӽǷ������ Epic Games �����ӽǷ��򡪡�Ҳ���Ǿ��淴�䷽�򡪡����ǵ���������������o��������һ�����ơ�
    //������Ԥ���˵Ļ��������Ͳ���Ҫ�����ӽǷ����ˡ�����ζ�ŵ�������ͼ�ĽǶȹ۲����ľ��淴��ʱ���õ����ӽǾ��淴��Ч�����Ǻܺã�ͼƬ�������¡�Moving Frostbite to PBR������Ȼ����ͨ��������Ϊ����һ���������Э��
    vec3 R = N;
    vec3 V = R;
    
    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        //ʹ�õͲ��� Hammersley ���к�����������������ɷ��������ǿ����������Ԥ�˲���������ɫ����
        //����һ��ƫ������ѡ���뷽�����������(��Ҫ�Բ���)��
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            //Ԥ���˾������������⡪�����ھ��淴���й�ǿ�ȵı仯�󣬸�Ƶϸ�ڶ࣬���ԶԾ��淴����о�����Ҫ����������������ȷ��ӳ HDR ��������Ļ��ұ仯��
            //�����Ѿ������˴����Ĳ�����������ĳЩ�����£���ĳЩ�ϴֲڵ� mip �����Ͽ�����Ȼ��������������������Χ���ֵ�״ͼ����
            
            //һ�ֽ�������ǽ�һ��������������������ĳЩ����»��ǲ�����
            //��һ�ַ����� Chetan Jags ���������ǿ�����Ԥ���˾���ʱ����ֱ�Ӳ���������ͼ�����ǻ��ڻ��ֵ� PDF �ʹֲڶȲ���������ͼ�� mipmap ���Լ���α��
            float D   = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; //source cubemap�ķֱ���(ÿ��)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}