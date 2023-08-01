#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{		
    //// The world vector acts as the normal of a tangent surface from the origin, aligned to WorldPos. Given this normal, calculate all incoming radiance of the environment. The result of this radiance is the radiance of light coming from -Normal direction, which is what we use in the PBR shader to sample irradiance.
    //世界向量作为从原点到WorldPos的切线面的法线。给定这个法线，计算环境的所有入射辐射度。
    //这个辐射度的结果是来自法线方向的光的辐射度，我们在PBR着色器中使用它来采样辐照度。
    vec3 N = normalize(WorldPos);

    vec3 irradiance = vec3(0.0);   
    
     // 从原点计算切线空间
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
    //有很多方法可以对环境贴图进行卷积，但是对于本教程，我们的方法是：对于立方体贴图的每个纹素
    //，在纹素所代表的方向的半球 Ω内生成固定数量的采样向量，并对采样结果取平均值。数量固定的采样向量将均匀地分布在半球内部。 
    float sampleDelta = 0.025;
    float nrSamples = 0.0f;
    //遍历半球
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // 球面到笛卡儿(切线空间)
            //获取一个球面坐标并将它们转换为 3D 直角坐标向量，
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // 将向量从切线空间转换为世界空间
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 
            //我们将采样的颜色值乘以系数 cos(θ) ，因为较大角度的光较弱，而系数 sin(θ) 则用于权衡较高半球区域的较小采样区域的贡献度。
            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}