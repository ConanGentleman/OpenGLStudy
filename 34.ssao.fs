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

// 基于屏幕尺寸除以噪声大小的平铺（Tile）噪声纹理
//注意我们这里有一个noiseScale的变量。我们想要将噪声纹理平铺(Tile)在屏幕上，
//但是由于TexCoords的取值在0.0和1.0之间，texNoise纹理将不会平铺。
//所以我们将通过屏幕分辨率除以噪声纹理大小的方式计算TexCoords的缩放大小，并在之后提取相关输入向量的时候使用。
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0); 

uniform mat4 projection;
//我们需要沿着表面法线方向生成大量的样本。就像我们在这个教程的开始介绍的那样，我们想要生成形成半球形的样本。
//但由于对每个表面法线方向生成采样核心非常困难，也不合实际，

//因此我们将在切线空间(Tangent Space)内生成采样核心，法向量将指向正z方向
void main()
{
    // 获取SSAO算法的输入
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);//试了一下rgb改成xyz结果好像没太大变化？
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
    // 创建TBN基变换矩阵:从切线空间到观察空间
    //通过使用一个叫做Gramm-Schmidt处理(Gramm-Schmidt Process)的过程，我们创建了一个正交基(Orthogonal Basis)，
    //每一次它都会根据randomVec的值稍微倾斜。注意因为我们使用了一个随机向量来构造切线向量，
    //我们没必要有一个恰好沿着几何体表面的TBN矩阵，也就是不需要逐顶点切线(和双切)向量。
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // 迭代样本核心并计算遮挡因子
    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // 得到样本位置
        //对于每一次迭代我们首先变换各自样本到观察空间。
        //之后我们会加观察空间核心偏移样本到观察空间片段位置上；
        //最后再用radius乘上偏移样本来增加(或减少)SSAO的有效取样半径。
        vec3 samplePos = TBN * samples[i]; // 从切线空间到观察空间
        samplePos = fragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        //投影样本位置（到样本纹理）（获得屏幕上的位置/纹理）
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // 从观察空间转换到裁剪空间
        offset.xyz /= offset.w; // 透视划分
        offset.xyz = offset.xyz * 0.5 + 0.5; // 变换到0.0 - 1.0的值域
        
        //得到样本深度
        //我们使用offset向量的x和y分量采样线性深度纹理从而获取样本位置从观察者视角的深度值(第一个不被遮蔽的可见片段)。
        float sampleDepth = texture(gPosition, offset.xy).z; //得到核心样本的深度值
        
        // 范围检查和计算
        //我们接下来检查样本的当前深度值是否大于存储的深度值，如果是的，添加到最终的贡献因子上。

        //当检测一个靠近表面边缘的片段时，它将会考虑测试表面之下的表面的深度值；
        //这些值将会(不正确地)影响遮蔽因子。我们可以通过引入一个范围检测从而解决这个问题，正如下图所示(John Chapman的佛像)：
        //我们引入一个范围测试从而保证我们只当被测深度值在取样半径内时影响遮蔽因子
        //这里我们使用了GLSL的smoothstep函数，它非常光滑地在第一和第二个参数范围内插值了第三个参数。如果深度差因此最终取值在radius之间，它们的值将会光滑地根据下面这个曲线插值在0.0和1.0之间：
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        //如果我们使用一个在深度值在radius之外就突然移除遮蔽贡献的硬界限范围检测(Hard Cut-off Range Check)，我们将会在范围检测应用的地方看见一个明显的(很难看的)边缘。(描边的那种感觉）
        
        //我们接下来检查样本的当前深度值是否大于存储的深度值，如果是的，添加到最终的贡献因子上。
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    //最后一步，我们需要将遮蔽贡献根据核心的大小标准化，并输出结果。
    //注意我们用1.0减去了遮蔽因子，以便直接使用遮蔽因子去缩放环境光照分量。
    occlusion = 1.0 - (occlusion / kernelSize);
    
    FragColor = occlusion;
}