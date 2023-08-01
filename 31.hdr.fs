#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure; //曝光参数

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    if(hdr)//使用色调映射算法
    {
        //Reinhard 色调映射算法平均地将所有亮度值分散到LDR上
        // vec3 result = hdrColor / (hdrColor + vec3(1.0));
        
        
        //曝光色调映射
        //如果我们有一个场景要展现日夜交替，我们当然会在白天使用低曝光，在夜间使用高曝光，
        //就像人眼调节方式一样。有了这个曝光参数，
        //我们可以去设置可以同时在白天和夜晚不同光照条件工作的光照参数，
        //我们只需要调整曝光参数就行了。
        vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
        // gamma矫正     
        result = pow(result, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
    else
    {
        vec3 result = pow(hdrColor, vec3(1.0 / gamma));
        FragColor = vec4(result, 1.0);
    }
}