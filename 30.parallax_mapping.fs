#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

uniform float heightScale;//引入一个heightScale的uniform，来进行一些额外的控制，因为视差效果如果没有一个缩放参数通常会过于强烈

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
    // 深度层数（用于陡峭视差映射）
    //陡峭视差映射的基本思想是将总深度范围划分为同一个深度/高度的多个层。从每个层中我们沿着P
    //方向移动采样纹理坐标，直到我们找到一个采样低于当前层的深度值
    const float minLayers = 8;
    const float maxLayers = 32;
    //我们可以通过对视差贴图的一个属性的利用，对算法进行一点提升。当垂直看一个表面的时候纹理时位移比以一定角度看时的小。
    //我们可以在垂直看时使用更少的样本，以一定角度看时增加样本数量：
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // 计算每一层的大小
    float layerDepth = 1.0 / numLayers;
    // 当前层级
    float currentLayerDepth = 0.0;
    // 每层的纹理坐标偏移量(从向量P开始)
    vec2 P = viewDir.xy / viewDir.z * heightScale;// x和y元素在切线空间中，viewDir向量除以它的z元素，用fragment的高度numLayers对它进行缩放
    vec2 deltaTexCoords = P / numLayers;//单位纹理偏移量
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
      
    //陡峭视差映射的基本思想是将总深度范围划分为同一个深度/高度的多个层。
    //从每个层中我们沿着P方向移动采样纹理坐标，直到我们找到一个采样低于当前层的深度值
    while(currentLayerDepth < currentDepthMapValue)
    {
        // 沿着P方向移动纹理坐标
        currentTexCoords -= deltaTexCoords;
        // 获取当前纹理坐标下的深度图值
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
        // 获取下一层的深度
        currentLayerDepth += layerDepth;  
    }
    //视差遮蔽映射
    // 在采样低于当前层的深度值（冲突）前 得到纹理坐标
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // 获取冲突前后的深度，用于线性插值
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // 纹理坐标插值
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main()
{           
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;
    
    texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);       
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    // obtain normal from normal map
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);   
   
    // get diffuse color
    vec3 color = texture(diffuseMap, texCoords).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular    
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}