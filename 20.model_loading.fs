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


////�����ɵ�Ч��������Ч����
//void main()
//{    
//    vec3 I = normalize(Position - cameraPos);//�۲�/�������������I����ʹ���������㷴������R
//    vec3 R = reflect(I, normalize(Normal));
//    vec3 reflectMap = vec3(texture(texture_reflection1, TexCoords));
//    vec3 reflection = vec3(texture(texture1, R).rgb) * reflectMap * 2;
//    //FragColor = texture(texture_diffuse1, TexCoords);
//    FragColor = vec4(texture(texture1, R).rgb, 1.0);//ʹ��R������պ���������ͼ�в���
//}
////����Ч��
//void main()
//{             
//    float ratio = 1.00 / 1.52;
//    vec3 I = normalize(Position - cameraPos);
//    vec3 R = refract(I, normalize(Normal), ratio);
//    FragColor = vec4(texture(texture1, R).rgb, 1.0);
//}

//void main(){
//    // ��������\n"
//       vec3 ambient = texture(texture_height1, TexCoords).rgb;//��������Ĳ�����ɫ����Ϊ�����������ɫͬ����ֵ��\n"
//    // ��������� \n"
//    vec3 I = normalize(Position - cameraPos);//�����߹�һ�� \n"
//    vec3 R = reflect(I, normalize(Normal));
//    vec3 diffuse = texture(texture_diffuse1, TexCoords).rgb;//�������в���Ƭ�ε���������ɫֵ,�õ���������Ĺ��� \n"
//    // specular\n"
//    I = normalize(Position - cameraPos);//�����߹�һ�� \n"
//    R = reflect(I, normalize(Normal));
//    vec3 specular = texture(texture_specular1, TexCoords).rgb; //���淴����� \n"
//    //���\n"
//    vec3 result = ambient + diffuse + specular;//�������������������������������� \n"
//    FragColor = vec4(result, 1.0);
//}