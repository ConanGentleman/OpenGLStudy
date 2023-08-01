#version 330 core
out vec2 FragColor;
in vec2 TexCoords;

const float PI = 3.14159265359;
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
	// from spherical coordinates to cartesian coordinates - halfway vector
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
//���� BRDF ����Ǿ��� IBL ���ֵ�һ���֣��������Ҫ�� Schlick-GGX ���κ�����ʹ�� kIBL
//��ע�⣬��Ȼ k���Ǵ� a ��������ģ�������� a ���� roughness ��ƽ��������ͬ����� a ���������������������������Ǽ�װƽ�����ˡ�
//�����ڱκ���
float GeometrySchlickGGX(float NdotV, float roughness)
{
    // ע�⣬����ΪIBLʹ���˲�ͬ��k
    float a = roughness;
    float k = (a * a) / 2.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
//BRDF �����ɫ���� 2D ƽ����ִ�м��㣬ֱ��ʹ���� 2D ����������Ϊ������루NdotV �� roughness����
//������Ԥ�˲����ľ������������ƣ���֮ͬ�����ڣ������ڸ��� BRDF �ļ��κ����� Fresnel-Schlick �������������������
vec2 IntegrateBRDF(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV*NdotV);
    V.y = 0.0;
    V.z = NdotV;

    float A = 0.0;
    float B = 0.0; 

    vec3 N = vec3(0.0, 0.0, 1.0);
    //�б��ھ��Ȼ�����أ��������ؿ��壩�ڻ��ְ��� �������������������ǵĲ�������ݴֲڶȣ�ƫ��΢����İ������ĺ�۷��䷽��
    //�������̽�������֮ǰ�����Ĺ������ƣ���ʼһ����ѭ��������һ��������Ͳ��죩����ֵ���ø�����ֵ�����߿ռ����������������������������任������ռ䲢�Գ����ķ���Ȳ�����
    //��֮ͬ�����ڣ���������ʹ�õͲ�������ֵ��Ϊ���������ɲ���������
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        //����һ��ƫ������ѡ���뷽�����������(��Ҫ�Բ���)��
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}
// ----------------------------------------------------------------------------
void main() 
{
    vec2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
    FragColor = integratedBRDF;
}