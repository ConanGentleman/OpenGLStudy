#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material parameters
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
//��ȡ����ռ�����߷��ߵļ򵥼��ɣ��Ա���PBR����ļ�
//�����㲻��Ҳ���õ���;������Σ���ͨ����Ҫ��ͨ���ķ�ʽ����������ӳ��;
//�Ҽƻ��ڷ���ӳ��̳̺����ĳ���ط��������ּ�����
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
// ���㷨�߷ֲ�����D ����ʽ��PBR���۲��֣�
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
//���κ���G����ʽ��PBR���۲��֣�
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
//Ϊ����Ч�Ĺ��㼸�β��֣���Ҫ���۲췽�򣨼����ڱ�(Geometry Obstruction)���͹��߷���������������Ӱ(Geometry Shadowing)�������ǽ�ȥ��
//���ǿ���ʹ��ʷ��˹��(Smith��s method)�������߶��������У�
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
//����ֱ�۲��ʱ���κ�������߲��ʱ��涼��һ������������(Base Reflectivity) F0
//���������һ���ĽǶ���ƽ���Ͽ���ʱ�����з��ⶼ��������������
//���������̿�����Fresnel-Schlick���Ʒ���ý��ƽ�
//������������ΪFreh-nel�������������Ǳ�����Ĺ��߶Աȹ��߱�����Ĳ�����ռ�ı��ʣ�������ʻ��������ǹ۲�ĽǶȲ�ͬ����ͬ
//�������������ʺ������غ�ԭ�����ǿ���ֱ�ӵó����߱�����Ĳ����Լ�����ʣ���������
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
     //(1.0 - F0) ��ʾ�������䲿�ֺ�ʣ������䣨�����䣩���֣������غ㣩
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   
// ----------------------------------------------------------------------------
void main()
{		
    // material properties
    vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;
       
    // input lighting data
    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N); 

    //���㴹ֱ����ķ�����;������ʵ�(������)ʹ��F0 0.04������ǽ�����ʹ��F0�ķ�������ɫ(������������)
    //��PBR�����������Ǽ򵥵���Ϊ������ľ�Ե����F0Ϊ0.04��ʱ�������Ӿ�������ȷ��
    //���Կ��������ڷǽ�������F0ʼ��Ϊ0.04�����ڽ������棬���Ǹ��ݳ�ʼ��F0�ͱ��ֽ������Եķ����ʽ������Բ�ֵ��
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);    
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);        
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS���ڷ�����
        //F��ʾ���д�����������ϵľ��淴���Ĺ���
        vec3 kS = F;
        // Ϊ�������غ㣬�������;��淴��ⲻ�ܳ���1.0(���Ǳ��淢��);
        // Ϊ�˱������ֹ�ϵ����ɢ����(kD)Ӧ�õ���1.0 - kS��
        //��kS���Ǻ����׼�������ı�ֵkD
        //����kS        ��ʾ�����б�����������ı����� ��ʣ�µĹ��ܻᱻ���䣬 ��ֵ��ΪkD
        vec3 kD = vec3(1.0) - kS;
        //��kD����������ȣ�����ֻ�зǽ�������������⣬����������ֽ����������Ի��(������û���������)��
        kD *= 1.0 - metallic; 

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        //���յĽ��Lo������˵�ǳ�����ߵķ����ʣ�ʵ�����Ƿ����ʷ��̵��ڰ������򦸵Ļ��ֵĽ��
        //��������ʵ���ϲ���Ҫȥ�������Ϊ�������п��ܵ�������߷�������֪��ֻ��4������(�趨����4����Դ����������߻�Ӱ��Ƭ�ε���ɫ
        //c��ʾ������ɫ������һ��������������������Ԧ���Ϊ�˶����������б�׼��(����)����Ϊǰ�溬��BRDF�Ļ��ַ������ܦ�Ӱ���
        //kD * albedo / PI ��                    ��ʾk_d*f_lambert��
        //specular��                             ��ʾk_s*f_cook-torrance��
        //(kD * albedo / PI + specular) :        ��ʾf_r
        //radiance��                             ��ʾL_i
        //NdotL��                                ��ʾn*w_i
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // ע�⣬�����Ѿ���BRDF�����˷�����(kS)���������ǲ����ٳ���kS
    }   
    
    // ambient lighting (we now use IBL as the ambient term)
    //��������(��������ʹ��IBL��Ϊ������)
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    // kS���ڷ�����
    //F��ʾ���д�����������ϵľ��淴���Ĺ���
    vec3 kS = F;
    // Ϊ�������غ㣬�������;��淴��ⲻ�ܳ���1.0(���Ǳ��淢��);
          // Ϊ�˱������ֹ�ϵ����ɢ����(kD)Ӧ�õ���1.0 - kS��
          //��kS���Ǻ����׼�������ı�ֵkD
          //����kS        ��ʾ�����б�����������ı����� ��ʣ�µĹ��ܻᱻ���䣬 ��ֵ��ΪkD
    vec3 kD = 1.0 - kS;
    //��kD����������ȣ�����ֻ�зǽ�������������⣬����������ֽ����������Ի��(������û���������)��
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;//���ն���ͼ����
    vec3 diffuse      = irradiance * albedo;//������
    
    //��Ԥ�˲�ӳ��ͼ��BRDF���ұ���в�����������Split-Sum���ƽ����������һ���Ի��IBL���沿�֡�
    const float MAX_REFLECTION_LOD = 4.0;
    //��Ԥ�˲�ӳ��ͼ����(���䷽�̵������䲿�� )
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb; 
    //�Ծ��淴����ֵ� BRDF ���ֲ���(���䷽�̵ļ�Ӿ��淴�䲿��)
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    //���˴� BRDF ������ͼ��Ԥ���˵Ļ�����ͼ���Ϳ��Խ����߽���������Ի�þ��淴����ֵĽ��
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    //specular û�г��� kS����Ϊ�Ѿ��˹��˷�����ϵ��
    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;

     // HDRɫ��ӳ��
    color = color / (color + vec3(1.0));
       // gamma����
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color , 1.0);
}