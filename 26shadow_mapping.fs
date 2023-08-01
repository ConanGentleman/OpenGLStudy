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
    // ͸�ӳ����������пռ�����ķ�Χ-w��wתΪ-1��1����Ҫ��x��y��zԪ�س���������wԪ����ʵ�֣�
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Ϊ�˺������ͼ�������Ƚϣ�z������Ҫ�任��[0,1]��Ϊ����Ϊ�������ͼ�в��������꣬xy����Ҳ��Ҫ�任��[0,1]
    projCoords = projCoords * 0.5 + 0.5;
    //���õ����λ����Ұ���������ȣ��ӹ��ߵĽǶȻ����������ֵ(ʹ��[0,1]��ΧfragPosLight��Ϊ����)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // �ӹ�ĽǶȻ�õ�ǰƬ�ε����
    float currentDepth = projCoords.z;
    // ����ƫ��(�������ͼ�ֱ��ʺ�б��)��������Ӱʧ�������Ե�������ʽ�����⡣
    //��Ϊ��Ӱ��ͼ�����ڷֱ��ʣ��ھ����Դ�Ƚ�Զ������£����Ƭ�ο��ܴ������ͼ��ͬһ��ֵ��ȥ����
    //���Ƭ�ξͻ��ͬһ��б�µ�������������в�������Щ�ڵذ����棬��Щ�ڵذ����棻�����������õ�����Ӱ�����˲��졣
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    //��鵱ǰfrag pos�Ƿ�����Ӱ��
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
        //���Ƭ�ζ�Ӧ��һ���������ء�������Ƕ��Ƭ�λ�������ͼ��ͬһ�����ֵ���в������⼸��Ƭ�α�õ�����ͬһ����Ӱ����ͻ������ݱߡ�
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; //һ���򵥵�PCF��ʵ���Ǽ򵥵Ĵ������������ܶ������ͼ������Ȼ��ѽ��ƽ��������
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // ����һ�����Ǻڰ�������������곬���˹��������׶��Զƽ�档����Կ�����Ƭ��ɫ�������ǳ����ڹ�Դ��׶�ļ�Զ����
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
    vec3 halfwayDir = normalize(lightDir + viewDir);  //�������
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // Calculate shadow
    float shadow = shadows ? ShadowCalculation(fs_in.FragPosLightSpace) : 0.0;                      
    shadow = min(shadow, 0.75); // ������Ӱ��ǿ��:����Ӱ��������һЩ������/�߹�
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0f);
}