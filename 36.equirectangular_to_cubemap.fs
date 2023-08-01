#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
//将等距柱状投影图整齐地折叠到立方体的每个面一样。
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
//而在片段着色器中，我们为立方体的每个部分着色，方法类似于将等距柱状投影图整齐地折叠到立方体的每个面一样。
//为了实现这一点，我们先获取片段的采样方向，这个方向是从立方体的局部坐标进行插值得到的，
//然后使用此方向向量和一些三角学魔法对等距柱状投影图进行采样，如同立方体图本身一样。我们直接将结果存储到立方体每个面的片段中
void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}