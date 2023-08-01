#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform bool shadows;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 透视除法（将裁切空间坐标的范围-w到w转为-1到1，这要将x、y、z元素除以向量的w元素来实现）
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 为了和深度贴图的深度相比较，z分量需要变换到[0,1]；为了作为从深度贴图中采样的坐标，xy分量也需要变换到[0,1]
    projCoords = projCoords * 0.5 + 0.5;
    //将得到光的位置视野下最近的深度，从光线的角度获得最近的深度值(使用[0,1]范围fragPosLight作为坐标)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 从光的角度获得当前片段的深度
    float currentDepth = projCoords.z;
    // 计算偏差(基于深度图分辨率和斜率)：处理阴影失真有明显的线条样式的问题。
    //因为阴影贴图受限于分辨率，在距离光源比较远的情况下，多个片段可能从深度贴图的同一个值中去采样
    //多个片段就会从同一个斜坡的深度纹理像素中采样，有些在地板上面，有些在地板下面；这样我们所得到的阴影就有了差异。
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    //检查当前frag pos是否在阴影中
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
        //多个片段对应于一个纹理像素。结果就是多个片段会从深度贴图的同一个深度值进行采样，这几个片段便得到的是同一个阴影，这就会产生锯齿边。
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; //一个简单的PCF的实现是简单的从纹理像素四周对深度贴图采样，然后把结果平均起来：
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // 仍有一部分是黑暗区域。那里的坐标超出了光的正交视锥的远平面。你可以看到这片黑色区域总是出现在光源视锥的极远处。
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.4);
    // Ambient
    vec3 ambient = 0.2 * color;
    // Diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // Specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  //半程向量
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // Calculate shadow
    float shadow = shadows ? ShadowCalculation(fs_in.FragPosLightSpace) : 0.0;                      
    shadow = min(shadow, 0.75); // 减少阴影的强度:在阴影区域允许一些漫反射/高光
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0f);
}