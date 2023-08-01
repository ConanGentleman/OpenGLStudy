#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = aTexCoords;
    //如果我们希望更精确的话就不要将TBN向量乘以model矩阵，
    //而是使用法线矩阵，因为我们只关心向量的方向，不关心平移和缩放
    mat3 normalMatrix = transpose(inverse(mat3(model)));//法线矩阵(法线矩阵就是这样求)
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);//格拉姆-施密特正交化过程,对TBN向量进行重正交化，这样每个向量就又会重新垂直了
    vec3 B = cross(N, T); //叉乘求副切线
    
    mat3 TBN = transpose(mat3(T, B, N));//转置    
    vs_out.TangentLightPos = TBN * lightPos; //将灯光位置从世界空间变换为切线空间
    vs_out.TangentViewPos  = TBN * viewPos; //变换为切线空间
    vs_out.TangentFragPos  = TBN * vs_out.FragPos;//变换为切线空间
        
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}