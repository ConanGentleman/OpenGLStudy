#define STB_IMAGE_IMPLEMENTATION
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "stb_image.h"
#include "camera.h"
#include<iostream>
#include <glm/gtx/string_cast.hpp>
using namespace std;
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoords;\n"

"out vec3 FragPos;\n"
"out vec3 Normal;\n "
"out vec2 TexCoords;\n"

"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n "
"void main()\n"
"{\n"
"   FragPos = vec3(model * vec4(aPos, 1.0)); //Ƭ�������������λ�ã��Ա������������շ���\n"
"   Normal = mat3(transpose(inverse(model))) * aNormal; //���ߵ�λ��ͨ��ģ�;�������Ͻ�3*3���ֵ�������ת�þ������任Ϊ���������µ�λ��\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);//gl_Position��OpenGL�ı�׼������������vec4\n"
"   TexCoords = aTexCoords;\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"struct Material {//�Ƴ��˻����������ɫ��������Ϊ��������ɫ�ڼ�����������¶�������������ɫ���������ǲ���Ҫ�����Ƿֿ����棺\n"
"    sampler2D diffuse; \n"
"    sampler2D specular; \n"
"    float shininess; \n"
"}; \n"
"struct Light {\n"
"    vec3 position; \n"
"    vec3 direction;\n"
"   float cutOff;\n"
"   float outerCutOff;\n "

"   vec3 ambient; \n"
"    vec3 diffuse; \n"
"    vec3 specular; \n"

"   //���Դ˥������\n"
"   float constant;\n"
"   float linear;\n"
"   float quadratic;\n"
"}; \n"
"in vec2 TexCoords;\n"
"in vec3 Normal;\n"
"in vec3 FragPos;\n"

"out vec4 FragColor;\n"

"uniform vec3 viewPos;//�ӽ�����λ�ã����λ�ã�\n"
"uniform Material material;\n"
"uniform Light light;\n"

"void main()\n"
"{\n"
"// ��������\n"
"   vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;//��������Ĳ�����ɫ����Ϊ�����������ɫͬ����ֵ��\n"

"// ��������� \n"
"vec3 norm = normalize(Normal);//�����߹�һ�� \n"
"vec3 lightDir = normalize(light.position - FragPos); //���շ���\n"
"float diff = max(dot(norm, lightDir), 0.0);//������뷨�ߵļн�cos \n"
"vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;//�������в���Ƭ�ε���������ɫֵ,�õ���������Ĺ��� \n"

"// specular\n"
"vec3 viewDir = normalize(viewPos - FragPos);//�ӽǷ��� \n"
"vec3 reflectDir = reflect(-lightDir, norm);//����߾������ߺ�ķ��䷽�� \n"
"float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);//�ӽ��뷴�䷽���cos \n"
" vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb; //���淴����� \n"

"// �۹�� (���Ե)\n"
"float theta = dot(lightDir, normalize(-light.direction));\n"
"float epsilon = (light.cutOff - light.outerCutOff);\n"
"float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);//clamp�����ѵ�һ������Լ������0.0��1.0֮�� \n"
"diffuse *= intensity; \n"
"specular *= intensity;// �����Ի���������Ӱ�죬������������һ��� \n"

"//˥��\n"
"   float distance    = length(light.position - FragPos);//�������Ƭ�εľ�����length���Դ��ĺ���\n"
"   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); //��ʽ����\n"
"   ambient *= attenuation;\n"
"   diffuse *= attenuation;\n"
"   specular *= attenuation;\n"

"//���\n"
"vec3 result = ambient + diffuse + specular;//�������������������������������� \n"
"FragColor = vec4(result, 1.0); \n"
"}\n\0";
const char* vertexShaderSourceLightcube = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n "
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);//gl_Position��OpenGL�ı�׼������������vec4\n"
"}\0";
const char* fragmentShaderSourceLightcube = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0);\n"
"}\n\0";
float mixValue = 0.2f;
float screenWidth = 800;
float screenHeight = 600;
float fov = 45.0f;
float aspect = 800.0f / 600.0f;
float deltaTime = 0.0f; // ��ǰ֡����һ֡��ʱ���
float lastFrame = 0.0f; // ��һ֡��ʱ��
bool firstMouse = true;
float yaw = -90.0f;	// ƫ������ʼ��Ϊ-90.0�ȣ���Ϊ0.0��ƫ������һ��ָ���ҵķ���ʸ���������������������תһ�㡣
float pitch = 0.0f;//������

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;//�����ʼ������

//������ͼ��ȡ
unsigned int loadTexture(const char* path);
// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
//glm::vec3 lightDirection(0.2f, 1.0f, 0.3f);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);//��Ļ�����¼�
void mouse_callback(GLFWwindow* window, double xpos, double ypos);//����ƶ��¼�
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//�����¼�
void processInput(GLFWwindow* window);//���������¼�
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
/*
ʹ��GLFW��glfwGetKey����������Ҫһ�������Լ�һ��������Ϊ���롣
����������᷵����������Ƿ����ڱ����¡�
���ǽ�����һ��processInput�����������е�������뱣������
���ؼ�(Esc)�����û�а��£�glfwGetKey���᷵��GLFW_RELEASE��
����û���ȷ�����˷��ؼ������ǽ�ͨ��glfwSetwindowShouldCloseʹ�ð�WindowShouldClose��������Ϊ true�ķ����ر�GLFW��
��һ��whileѭ����������⽫��ʧ�ܣ����򽫻�رա�
*/
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}
//���û��ı䴰�ڵĴ�С��ʱ���ӿ�ҲӦ�ñ����������ǿ��ԶԴ���ע��һ���ص�����(Callback Function)��
//������ÿ�δ��ڴ�С��������ʱ�򱻵��á�����ص�������ԭ������
//��Ҫһ��GLFWwindow��Ϊ���ĵ�һ���������Լ�����������ʾ���ڵ���ά�ȡ�ÿ�����ڸı��С��
//GLFW�������������������Ӧ�Ĳ������㴦��
//����Ҫע���������������GLFW����ϣ��ÿ�����ڵ�����С��ʱ��������������
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
glm::mat4 trans;
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
void setLight() {
    model = glm::mat4();
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
}
void transform_rotationOP() {
    trans = glm::mat4(1.0f);
    //trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));//��ת90�ȣ���z��
    //trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));//ÿ�������ŵ�0.5����

    //��λ������ת
    trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));//�ƶ������½�
    trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));//����ʱ����ת

    ////����ת��λ��
    //trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));//����ʱ����ת
    //trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));//�ƶ������½�
    //cout << trans.length() << endl;
}
void scaleOP() {
    trans = glm::mat4(1.0f);
    //trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));//��ת90�ȣ���z��
    //trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));//ÿ�������ŵ�0.5����
    trans = glm::translate(trans, glm::vec3(-0.5f, 0.5f, 0.0f));
    float scaleAmount = static_cast<float>(sin(glfwGetTime()));
    //��ʱ������
    trans = glm::scale(trans, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
    ////����ת��λ��
    //trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));//����ʱ����ת
    //trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));//�ƶ������½�
    //cout << trans.length() << endl;
}
void modelmat() {
    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));//��������x����ת��ʹ������������ڵ���һ��
    //model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
}
void viewmat() {
    view = glm::mat4(1.0f);
    // ע�⣬���ǽ�����������Ҫ�����ƶ������ķ������ƶ���
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
}
void projectionmat() {
    projection = glm::mat4(1.0f);
    //projection = glm::perspective(glm::radians(45.0f), screenWidth / screenHeight, 0.1f, 100.0f);//��һ������������fov��ֵ������ʾ������Ұ(Field of View),ͨ������Ϊ45.0f.�ڶ������������˿�߱ȣ����ӿڵĿ���Ը����á������͵��ĸ�����������ƽ��ͷ��Ľ���Զƽ��
    projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);//��һ������������fov��ֵ������ʾ������Ұ(Field of View),ͨ������Ϊ45.0f.�ڶ������������˿�߱ȣ����ӿڵĿ���Ը����á������͵��ĸ�����������ƽ��ͷ��Ľ���Զƽ��
}
//LookAt����
glm::mat4 calculate_lookAt_matrix(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp)
{
    // 1. Position = known
    // 2. Calculate cameraDirection
    glm::vec3 zaxis = glm::normalize(position - target);
    // 3. Get positive right axis vector
    glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(worldUp), zaxis));
    // 4. Calculate camera up vector
    glm::vec3 yaxis = glm::cross(zaxis, xaxis);

    // Create translation and rotation matrix
    // In glm we access elements as mat[col][row] due to column-major layout
    glm::mat4 translation = glm::mat4(1.0f); // Identity matrix by default
    translation[3][0] = -position.x; // Third column, first row
    translation[3][1] = -position.y;
    translation[3][2] = -position.z;
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0][0] = xaxis.x; // First column, first row
    rotation[1][0] = xaxis.y;
    rotation[2][0] = xaxis.z;
    rotation[0][1] = yaxis.x; // First column, second row
    rotation[1][1] = yaxis.y;
    rotation[2][1] = yaxis.z;
    rotation[0][2] = zaxis.x; // First column, third row
    rotation[1][2] = zaxis.y;
    rotation[2][2] = zaxis.z;

    // Return lookAt matrix as combination of translation and rotation matrix
    return rotation * translation; // Remember to read from right to left (first translation then rotation)
}

int main()
{


    glfwInit();//��ʼ��glfw
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint�����ĵ�һ����������ѡ������ƣ����ǿ��ԴӺܶ���GLFW_��ͷ��ö��ֵ��ѡ��
    //�ڶ�����������һ�����ͣ������������ѡ���ֵ
    // GLFW_CONTEXT_VERSION_MAJOR��ʾ��ѡ�ͻ��� API ���κ���Ч���汾�ţ�OpenGL�İ汾Ϊ3.3 �������汾�� Ϊ3��
    //�ú��������е�ѡ���Լ���Ӧ��ֵ�������� GLFW��s window handling(https://www.glfw.org/docs/latest/window.html#window_hints) ��ƪ�ĵ����ҵ�
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // GLFW_CONTEXT_VERSION_MINOR��ʾ��ѡ�ͻ��� API ���κ���Ч�ΰ汾�ţ�OpenGL�İ汾Ϊ3.3 ���Դΰ汾�� Ϊ3��
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //GLFW_OPENGL_PROFILE��ʾ������ʹ�õ� OpenGL �����ļ�,ֵΪGLFW_OPENGL_CORE_PROFILE��������GLFW����ʹ�õ��Ǻ���ģʽ(Core-profile)
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);//���ʹ�õ���Mac OS Xϵͳ���㻹��Ҫ�����д��뵽��ĳ�ʼ����������Щ���ò���������

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);//֪ͨGLFW�����Ǵ��ڵ�����������Ϊ��ǰ�̵߳�����������
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//ע��ص�����
    glfwSetCursorPosCallback(window, mouse_callback);//ע��ص�����
    glfwSetScrollCallback(window, scroll_callback);//ע��ص�����
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//����GLFW�������ǵ����
    //GLAD����������OpenGL�ĺ���ָ��ģ�
    //�����ڵ����κ�OpenGL�ĺ���֮ǰ������Ҫ��ʼ��GLAD
    //��GLAD��������������ϵͳ��ص�OpenGL����ָ���ַ�ĺ�����GLFW�����ǵ���glfwGetProcAddress
    //�����������Ǳ����ϵͳ��������ȷ�ĺ���
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //OpenGL��Ⱦ���ڵĳߴ��С�����ӿ�(Viewport)
    //glViewport����ǰ�����������ƴ������½ǵ�λ�á��������͵��ĸ�����������Ⱦ���ڵĿ�Ⱥ͸߶�
    //����Ҳ���Խ��ӿڵ�ά������Ϊ��GLFW��ά��С��������֮�����е�OpenGL��Ⱦ������һ����С�Ĵ�������ʾ��
    //�����ӵĻ�����Ҳ���Խ�һЩ����Ԫ����ʾ��OpenGL�ӿ�֮��
    //OpenGL���귶ΧֻΪ-1��1����(-1��1)��Χ�ڵ�����ӳ�䵽(0, 800)��(0, 600)
    glViewport(0, 0, 800, 600);

    //���ڱ���һ����ʾ��ʱ��framebuffer_size_callbackҲ�ᱻ���á�
    //��������Ĥ(Retina)��ʾ����width��height�������Ա�ԭ����ֵ����һ��
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


    glEnable(GL_DEPTH_TEST);
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };
    //ʮ�����ӵ�λ��
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    // 2. ���ö�������ָ��
    unsigned int vertexShader;//(https://learnopengl-cn.github.io/01%20Getting%20started/04%20Hello%20Triangle/)
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);//������ɫ��Դ��

    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)//�ж���ɫ���Ƿ����ɹ�
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int vertexShaderLightcube;//(https://learnopengl-cn.github.io/01%20Getting%20started/04%20Hello%20Triangle/)
    vertexShaderLightcube = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderLightcube, 1, &vertexShaderSourceLightcube, NULL);
    glCompileShader(vertexShaderLightcube);//������ɫ��Դ��

    glGetShaderiv(vertexShaderLightcube, GL_COMPILE_STATUS, &success);
    if (!success)//�ж���ɫ���Ƿ����ɹ�
    {
        glGetShaderInfoLog(vertexShaderLightcube, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    unsigned int fragmentShaderLightcube;
    fragmentShaderLightcube = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderLightcube, 1, &fragmentShaderSourceLightcube, NULL);
    glCompileShader(fragmentShaderLightcube);
    glGetShaderiv(fragmentShaderLightcube, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderLightcube, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }


    // 0. ���ƶ������鵽�����й�OpenGLʹ��
    unsigned int VBO, cubeVAO;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // λ������
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //���㷨������
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //// ��ɫ����
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    // ��������
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // ����ֻ��Ҫ�󶨵�VBO(��������glVertexAttribPointer)������Ҫ�����;VBO�������Ѿ�������������Ҫ����������(���Ѿ����ˣ������ڽ���Ŀ�ģ������ٴ�������)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();//�������
    glAttachShader(shaderProgram, vertexShader);//������Ķ�����ɫ�����ӵ��������
    glAttachShader(shaderProgram, fragmentShader);//�������ƬԪ��ɫ�����ӵ��������
    glLinkProgram(shaderProgram);//����������(Link)Ϊһ����ɫ���������
    // 3. ��������Ⱦһ������ʱҪʹ����ɫ������
    //glUseProgram(shaderProgram1);//����������

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    unsigned int shaderProgramLightcube;
    shaderProgramLightcube = glCreateProgram();//�������
    glAttachShader(shaderProgramLightcube, vertexShaderLightcube);//������Ķ�����ɫ�����ӵ��������
    glAttachShader(shaderProgramLightcube, fragmentShaderLightcube);//�������ƬԪ��ɫ�����ӵ��������
    glLinkProgram(shaderProgramLightcube);//����������(Link)Ϊһ����ɫ���������
    // 3. ��������Ⱦһ������ʱҪʹ����ɫ������
    //glUseProgram(shaderProgram1);//����������

    glDeleteShader(vertexShaderLightcube);
    glDeleteShader(fragmentShaderLightcube);

    glUseProgram(shaderProgramLightcube);//����������

    //glBindBuffer(GL_ARRAY_BUFFER, 0);


    //����Ϊ�߿�ģʽ��������䣬������glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)�������û�Ĭ��ģʽ������ֱ��ע������һ�еĴ���
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //�������֮������VAO�İ󶨣���������VAO���þͲ���������޸����VAO��������������ٷ������޸�����
    // VAOs������ζ���Ҫ����glBindVertexArray�����Ե���ֱ����Ҫʱ������ͨ������ȡ����VAOs(Ҳ����ȡ����vbo)��
    glBindVertexArray(0);

    /*
    ϣ�����������������ر���֮ǰ���ϻ���ͼ���ܹ������û����롣
    ��ˣ�������Ҫ�ڳ��������һ��whileѭ�������ǿ��԰�����֮Ϊ��Ⱦѭ��(Render Loop)��
    ������������GLFW�˳�ǰһֱ�������С����漸�еĴ����ʵ����һ���򵥵���Ⱦѭ����

    glfwWindowShouldClose����������ÿ��ѭ���Ŀ�ʼǰ���һ��GLFW�Ƿ�Ҫ���˳���
    ����ǵĻ��ú�������trueȻ����Ⱦѭ��������ˣ�֮��Ϊ���ǾͿ��Թر�Ӧ�ó����ˡ�
    glfwPollEvents���������û�д���ʲô�¼�������������롢����ƶ��ȣ������´���״̬��
    �����ö�Ӧ�Ļص�����������ͨ���ص������ֶ����ã���
    glfwSwapBuffers�����ύ����ɫ���壨����һ��������GLFW����ÿһ��������ɫֵ�Ĵ󻺳壩��
    ������һ�����б��������ƣ����ҽ�����Ϊ�����ʾ����Ļ�ϡ�
    */
    string diffuseMappath = "container2.png";
    string specularMappath = "container2_specular.png";
    //string numberMappath = "matrix.jpg";
    unsigned int diffuseMap = loadTexture(diffuseMappath.c_str());
    unsigned int specularMap = loadTexture(specularMappath.c_str());
    //unsigned int numberMap = loadTexture(numberMappath.c_str());

    glUseProgram(shaderProgram);
    int transLoc = glGetUniformLocation(shaderProgram, "material.diffuse");
    glUniform1i(transLoc, 0);//���������GL_TEXTURE0���͸�sampler2D��ֵΪ0
    transLoc = glGetUniformLocation(shaderProgram, "material.specular");
    //glUniform3f(transformLoc, 0.5f, 0.5f, 0.5f);
    glUniform1i(transLoc, 1);
    while (!glfwWindowShouldClose(window))
    {

        //����
        processInput(window);//ѭ��ʱ����Ƿ��¼�
        //��Ⱦ
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);//������Ļ��ɫ
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//��ɫ�����Ļ|�����Ȼ���
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, texture);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture1);
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glUseProgram(shaderProgram);//����������

        ////�ù��������ƶ�
        //lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
        //lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;
        int transformLoc;
        glm::vec3 lightColor;
        //transformLoc = glGetUniformLocation(shaderProgram, "lightColor");
        //glUniform3f(transformLoc, 1.0f, 1.0f, 1.0f);
        transformLoc = glGetUniformLocation(shaderProgram, "light.position");
        glUniform3f(transformLoc, camera.Position.x, camera.Position.y, camera.Position.z);//�۹��λ�þ����ӽ�
        transformLoc = glGetUniformLocation(shaderProgram, "light.direction");
        glUniform3f(transformLoc, camera.Front.x, camera.Front.y, camera.Front.z);
        transformLoc = glGetUniformLocation(shaderProgram, "light.cutOff");
        glUniform1f(transformLoc, glm::cos(glm::radians(12.5f)));
        transformLoc = glGetUniformLocation(shaderProgram, "light.outerCutOff");
        glUniform1f(transformLoc, glm::cos(glm::radians(17.5f)));
        transformLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniform3f(transformLoc, camera.Position.x, camera.Position.y, camera.Position.z);
        //lightColor.x = static_cast<float>(sin(glfwGetTime() * 2.0));
        //lightColor.y = static_cast<float>(sin(glfwGetTime() * 0.7));
        //lightColor.z = static_cast<float>(sin(glfwGetTime() * 1.3));

        //cout << camera.Zoom << endl;
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // ����Ӱ��
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // �ܵ͵�Ӱ��
        transformLoc = glGetUniformLocation(shaderProgram, "light.ambient");
        //glUniform3f(transformLoc, ambientColor.x, ambientColor.y, ambientColor.z);
        glUniform3f(transformLoc, 0.1f, 0.1f, 0.1f);
        transformLoc = glGetUniformLocation(shaderProgram, "light.diffuse");
        //glUniform3f(transformLoc, diffuseColor.x, diffuseColor.y, diffuseColor.z);
        glUniform3f(transformLoc, 0.8f, 0.8f, 0.8f);
        transformLoc = glGetUniformLocation(shaderProgram, "light.specular");
        glUniform3f(transformLoc, 1.0f, 1.0f, 1.0f);
        transformLoc = glGetUniformLocation(shaderProgram, "light.constant");
        glUniform1f(transformLoc, 1.0f);
        transformLoc = glGetUniformLocation(shaderProgram, "light.linear");
        glUniform1f(transformLoc, 0.09f);
        transformLoc = glGetUniformLocation(shaderProgram, "light.quadratic");
        glUniform1f(transformLoc, 0.032f);

        //transformLoc = glGetUniformLocation(shaderProgram, "material.ambient");
        ////glUniform3f(transformLoc, 1.0f, 0.5f, 0.31f);
        //glUniform3f(transformLoc, 0.0f, 0.1f, 0.06f);
        //transformLoc = glGetUniformLocation(shaderProgram, "material.diffuse");
        ////glUniform3f(transformLoc, 1.0f, 0.5f, 0.31f);
        //glUniform3f(transformLoc, 0.0f, 0.50980392f, 0.50980392f);
        //transformLoc = glGetUniformLocation(shaderProgram, "material.specular");
        ////glUniform3f(transformLoc, 0.5f, 0.5f, 0.5f);
        //glUniform3f(transformLoc, 0.50196078f, 0.50196078f, 0.50196078f);
        transformLoc = glGetUniformLocation(shaderProgram, "material.shininess");
        glUniform1f(transformLoc, 32.0f);


        projection = glm::perspective(glm::radians(camera.Zoom), (float)800.0 / (float)600.0, 0.1f, 100.0f);
        view = camera.GetViewMatrix();


        transformLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));
        //view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        transformLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));


        model = glm::mat4(1.0f);
        //modelmat();
        transformLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));
        // ����������ͼ
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        //�󶨾��淴����ͼ
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        ////�����ַ������ͼ
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, numberMap);

        glBindVertexArray(cubeVAO);
        //����ʮ������
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            transformLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }



        //���ƹ�Դ������
        glUseProgram(shaderProgramLightcube);//����������
        transformLoc = glGetUniformLocation(shaderProgramLightcube, "projection");
        if (transformLoc != -1) {
            //cout << -1 << endl;
            //transform_rotationOP();
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));

        }
        transformLoc = glGetUniformLocation(shaderProgramLightcube, "view");
        if (transformLoc != -1) {
            //cout << -1 << endl;
            //transform_rotationOP();
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));

        }
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        transformLoc = glGetUniformLocation(shaderProgramLightcube, "model");
        if (transformLoc != -1) {
            //cout << -1 << endl;
            //transform_rotationOP();
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));

        }
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // ��ѡ:һ����Դ��������;����ȡ������������Դ:
// ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderProgramLightcube);
    //��Ⱦѭ�����ͷ�������Դ
    glfwTerminate();
    return 0;
}
unsigned int loadTexture(char const* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1) format = GL_RED;
        if (nrComponents == 3) format = GL_RGB;
        if (nrComponents == 4) format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//�����зŴ�(Magnify)����С(Minify)������ʱ���������������˵�ѡ��
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);//�ͷ�ͼ���ڴ�
    return textureID;
}