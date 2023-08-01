#version 330 core
out vec4 FragColor;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_reflection1;

uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube texture1;


////铬做成的效果（反射效果）
//void main()
//{    
//    vec3 I = normalize(Position - cameraPos);//观察/摄像机方向向量I，并使用它来计算反射向量R
//    vec3 R = reflect(I, normalize(Normal));
//    vec3 reflectMap = vec3(texture(texture_reflection1, TexCoords));
//    vec3 reflection = vec3(texture(texture1, R).rgb) * reflectMap * 2;
//    //FragColor = texture(texture_diffuse1, TexCoords);
//    FragColor = vec4(texture(texture1, R).rgb, 1.0);//使用R来从天空盒立方体贴图中采样
//}
////折射效果
//void main()
//{             
//    float ratio = 1.00 / 1.52;
//    vec3 I = normalize(Position - cameraPos);
//    vec3 R = refract(I, normalize(Normal), ratio);
//    FragColor = vec4(texture(texture1, R).rgb, 1.0);
//}

//void main(){
//    // 环境光照\n"
//       vec3 ambient = texture(texture_height1, TexCoords).rgb;//将环境光的材质颜色设置为漫反射材质颜色同样的值。\n"
//    // 漫反射光照 \n"
//    vec3 I = normalize(Position - cameraPos);//将法线归一化 \n"
//    vec3 R = reflect(I, normalize(Normal));
//    vec3 diffuse = texture(texture_diffuse1, TexCoords).rgb;//从纹理中采样片段的漫反射颜色值,得到漫反射项的光照 \n"
//    // specular\n"
//    I = normalize(Position - cameraPos);//将法线归一化 \n"
//    R = reflect(I, normalize(Normal));
//    vec3 specular = texture(texture_specular1, TexCoords).rgb; //镜面反射分量 \n"
//    //结果\n"
//    vec3 result = ambient + diffuse + specular;//将环境光照与漫反射项体现在物体上 \n"
//    FragColor = vec4(result, 1.0);
//}