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
    // ��ȡƬ��λ�ú͹�λ��֮�������������������
    vec3 fragToLight = fragPos - lightPos;
    //ʹ��Ƭ�ι������������ͼ�в���
    //closedepth = texture(depthMap, fragToLight).r;
    //��Ŀǰ��[0,1]֮������Է�Χ�ڣ����������½���ת����ԭʼ���ֵ
    //������*= far_plane;
    //���ڻ��Ƭ�κ͹�Դλ��֮��ĵ�ǰ�������
    float currentDepth = length(fragToLight);//ֱ��ʹ��length������ȡ��ǰfragment�͹�Դ֮������ֵ
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
    // shadow /= (samples * samples * samples);//ȡƽ��
    
    //���ʹ�������򵥵�PCF�㷨����samples����Ϊ4.0ʱ��ÿ��fragment���ǻ�õ��ܹ�64����������̫���ˣ�
    //�������Щ�������Ƕ����(��shadow += 1.0;���ᱻ����)��������ԭʼ�������������������������ڲ������������Ĵ�ֱ������в�����������
    //���ǣ�û�У��򵥵ģ���ʽ�ܹ�ָ����һ���ӷ����Ƕ���ģ�
    //�и����ɿ���ʹ�ã���һ��ƫ������������gridSamplingDisk�����ǲ�඼�Ƿֿ��ģ�ÿһ��ָ����ȫ��ͬ�ķ����޳��˴˽ӽ�����Щ�ӷ���
    float shadow = 0.0;
    float bias = 0.15;//һ����Ӱƫ��bias��������Ӱʧ��
    int samples = 20;//PCF��������
    float viewDistance = length(viewPos - fragPos);
    //���ڹ۲�����һ��fragment�ľ������ı�diskRadius���������Ǿ��ܸ��ݹ۲��ߵľ���������ƫ�ư뾶��
    //���������Զ��ʱ����Ӱ����ͣ������˾͸�����
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;//�޳��˴˽ӽ�����Щ�ӷ������Ӱֵ
        closestDepth *= far_plane;   // ��Ŀǰ��[0,1]֮������Է�Χ�ڣ�*= far_plane���½���ת����ԭʼ���ֵ
        if(currentDepth - bias > closestDepth)//һ����Ӱƫ��bias��������Ӱʧ��
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