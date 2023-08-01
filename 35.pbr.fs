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

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
//获取世界空间的切线法线的简单技巧，以保持PBR代码的简化
//如果你搞不懂也不用担心;无论如何，你通常想要以通常的方式进行正常的映射;
//我计划在法线映射教程后面的某个地方记下这种技术。
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;//转换范围

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
// 计算法线分布函数D （公式见PBR理论部分）
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
//几何函数G（公式见PBR理论部分）
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
//为了有效的估算几何部分，需要将观察方向（几何遮蔽(Geometry Obstruction)）和光线方向向量（几何阴影(Geometry Shadowing)）都考虑进去。
//我们可以使用史密斯法(Smith’s method)来把两者都纳入其中：
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
//当垂直观察的时候，任何物体或者材质表面都有一个基础反射率(Base Reflectivity) F0
//但是如果以一定的角度往平面上看的时候所有反光都会变得明显起来。
//菲涅尔方程可以用Fresnel-Schlick近似法求得近似解
//菲涅尔（发音为Freh-nel）方程描述的是被反射的光线对比光线被折射的部分所占的比率，这个比率会随着我们观察的角度不同而不同
//利用这个反射比率和能量守恒原则，我们可以直接得出光线被折射的部分以及光线剩余的能量。
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    //(1.0 - F0) 表示除开反射部分后剩余的折射（漫反射）部分（能量守恒）
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void main()
{		
    vec3 albedo     = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    float metallic  = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao        = texture(aoMap, TexCoords).r;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    //计算垂直入射的反射率;如果介质电(如塑料)使用F0 0.04，如果是金属，使用F0的反照率颜色(金属工作流程)
    //在PBR金属流中我们简单地认为大多数的绝缘体在F0为0.04的时候看起来视觉上是正确的
    //可以看到，对于非金属表面F0始终为0.04。对于金属表面，我们根据初始的F0和表现金属属性的反射率进行线性插值。
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);//插值

    // 反射系数方程
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        //为了满足反射率方程，我们循环遍历每一个光源，计算他们各自的辐射率然后求和，
        //接着根据BRDF和光源的入射角来缩放该辐射率
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L); //半程向量
        float distance = length(lightPositions[i] - WorldPos);//距离
        float attenuation = 1.0 / (distance * distance);//衰减
        vec3 radiance = lightColors[i] * attenuation;//辐射率

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   //法线分布函数
        float G   = GeometrySmith(N, V, L, roughness);  //几何遮蔽函数    
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0); //菲涅尔方程 F表示所有打在物体表面上的镜面反射光的贡献
           
        vec3 numerator    = NDF * G * F; //BRDF公式分子
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; ////BRDF公式分母 加上0.0001 避免除以0
        vec3 specular = numerator / denominator; //Cook-Torrance specular BRDF项
        
        // kS等于菲涅耳
        //F表示所有打在物体表面上的镜面反射光的贡献
        vec3 kS = F;
        // 为了能量守恒，漫反射光和镜面反射光不能超过1.0(除非表面发光);
        // 为了保持这种关系，扩散分量(kD)应该等于1.0 - kS。
        //从kS我们很容易计算折射的比值kD
        //看作kS        表示光能中被反射的能量的比例， 而剩下的光能会被折射， 比值即为kD
        vec3 kD = vec3(1.0) - kS;
        //将kD乘以逆金属度，这样只有非金属具有漫反射光，或者如果部分金属，则线性混合(纯金属没有漫反射光)。
        kD *= 1.0 - metallic; 

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        //最终的结果Lo，或者说是出射光线的辐射率，实际上是反射率方程的在半球领域Ω的积分的结果
        //但是我们实际上不需要去求积，因为对于所有可能的入射光线方向我们知道只有4个方向(设定的有4个光源）的入射光线会影响片段的着色
        //c表示表面颜色（回想一下漫反射表面纹理）。除以π是为了对漫反射光进行标准化(半球)，因为前面含有BRDF的积分方程是受π影响的
        //kD * albedo / PI ：                    表示k_d*f_lambert项
        //specular：                             表示k_s*f_cook-torrance项
        //(kD * albedo / PI + specular) :        表示f_r
        //radiance：                             表示L_i
        //NdotL：                                表示n*w_i
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // 注意，我们已经将BRDF乘以了菲涅耳(kS)，所以我们不会再乘以kS
    }   
    
    //环境照明(请注意，下一个IBL教程将用environment lighting取代ambient lighting)。
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR色调映射
    color = color / (color + vec3(1.0));
    // gamma矫正
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}