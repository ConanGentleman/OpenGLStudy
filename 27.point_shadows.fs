#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;


// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float ShadowCalculation(vec3 fragPos)
{
    // 获取片段位置和光位置之间的向量，用于求出深度
    vec3 fragToLight = fragPos - lightPos;
    //使用片段光照向量从深度图中采样
    //closedepth = texture(depthMap, fragToLight).r;
    //它目前在[0,1]之间的线性范围内，让我们重新将它转换回原始深度值
    //最近深度*= far_plane;
    //现在获得片段和光源位置之间的当前线性深度
    float currentDepth = length(fragToLight);//直接使用length函数获取当前fragment和光源之间的深度值
    // test for shadows
    // float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    // float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;
    // PCF
    // float shadow = 0.0;
    // float bias = 0.05; 
    // float samples = 4.0;
    // float offset = 0.1;
    // for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    // {
        // for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        // {
            // for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            // {
                // float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r; // use lightdir to lookup cubemap
                // closestDepth *= far_plane;   // Undo mapping [0;1]
                // if(currentDepth - bias > closestDepth)
                    // shadow += 1.0;
            // }
        // }
    // }
    // shadow /= (samples * samples * samples);//取平均
    
    //如果使用上述简单的PCF算法，当samples设置为4.0时，每个fragment我们会得到总共64个样本，这太多了！
    //大多数这些采样都是多余的(即shadow += 1.0;不会被调用)，与其在原始方向向量附近处采样，不如在采样方向向量的垂直方向进行采样更有意义
    //可是，没有（简单的）方式能够指出哪一个子方向是多余的，
    //有个技巧可以使用，用一个偏移量方向数组gridSamplingDisk，它们差不多都是分开的，每一个指向完全不同的方向，剔除彼此接近的那些子方向
    float shadow = 0.0;
    float bias = 0.15;//一个阴影偏移bias，避免阴影失真
    int samples = 20;//PCF样本数量
    float viewDistance = length(viewPos - fragPos);
    //基于观察者里一个fragment的距离来改变diskRadius；这样我们就能根据观察者的距离来增加偏移半径了
    //，当距离更远的时候阴影更柔和，更近了就更锐利
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;//剔除彼此接近的那些子方向的阴影值
        closestDepth *= far_plane;   // 它目前在[0,1]之间的线性范围内，*= far_plane重新将它转换回原始深度值
        if(currentDepth - bias > closestDepth)//一个阴影偏移bias，避免阴影失真
            shadow += 1.0;
    }
    shadow /= float(samples);
        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = shadows ? ShadowCalculation(fs_in.FragPos) : 0.0;                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}