#define STB_IMAGE_IMPLEMENTATION
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include<iostream>
#include <glm/gtx/string_cast.hpp>
using namespace std;


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool blinn = false;
bool blinnKeyPressed = false;

unsigned int loadTexture(char const* path);
unsigned int loadCubemap(vector<std::string> faces);
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


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

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed)
    {
        blinn = !blinn;
        blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        blinnKeyPressed = false;
    }
}
//���û��ı䴰�ڵĴ�С��ʱ���ӿ�ҲӦ�ñ����������ǿ��ԶԴ���ע��һ���ص�����(Callback Function)��
//������ÿ�δ��ڴ�С��������ʱ�򱻵��á�����ص�������ԭ������
//��Ҫһ��GLFWwindow��Ϊ���ĵ�һ���������Լ�����������ʾ���ڵ���ά�ȡ�ÿ�����ڸı��С��
//GLFW�������������������Ӧ�Ĳ������㴦����
//����Ҫע���������������GLFW����ϣ��ÿ�����ڵ�����С��ʱ��������������
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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

    //stbi_set_flip_vertically_on_load(true);//����y�ᷭת

    //OpenGL��Ⱦ���ڵĳߴ��С�����ӿ�(Viewport)
    //glViewport����ǰ�����������ƴ������½ǵ�λ�á��������͵��ĸ�����������Ⱦ���ڵĿ��Ⱥ͸߶�
    //����Ҳ���Խ��ӿڵ�ά������Ϊ��GLFW��ά��С��������֮�����е�OpenGL��Ⱦ������һ����С�Ĵ�������ʾ��
    //�����ӵĻ�����Ҳ���Խ�һЩ����Ԫ����ʾ��OpenGL�ӿ�֮��
    //OpenGL���귶ΧֻΪ-1��1����(-1��1)��Χ�ڵ�����ӳ�䵽(0, 800)��(0, 600)
    glViewport(0, 0, 800, 600);

    //���ڱ���һ����ʾ��ʱ��framebuffer_size_callbackҲ�ᱻ���á�
    //��������Ĥ(Retina)��ʾ����width��height�������Ա�ԭ����ֵ����һ��
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ����stb_image.h��y���Ϸ�ת���ص�����(�ڼ���ģ��֮ǰ)��
    stbi_set_flip_vertically_on_load(true);

    //����ȫ��OpenGL״̬
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);//�������ģʽ
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//��Ϸ�ʽ
    //glEnable(GL_FRAMEBUFFER_SRGB);//�����Զ���gamma���������Ƚ���,���ⳡ�����ڣ���gammaУ������������ɫ�ռ�ת��Ϊ�����Կռ䡣����������֮ǰ�ͽ���gammaУ�������еĺ������������ڲ�������ȷ����ɫֵ��������Ҫ�Զ���gamma������
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CW);
    //glDepthFunc(GL_ALWAYS);//����ͨ����Ȳ��ԣ�������glDisable(GL_DEPTH_TEST);һ��

    //Shader shader("20.cubemaps.vs", "20.cubemaps.fs");
    Shader shader("24.advanced_lighting.vs", "24.advanced_lighting.fs");

    float planeVertices[] = {
        // positions            // normals         // texcoords
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
    };
    // plane VAO
    unsigned int planeVAO, planeVBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);

    unsigned int floorTexture = loadTexture("wood.png");

    while (!glfwWindowShouldClose(window))
    {

        //����
        processInput(window);//ѭ��ʱ����Ƿ��¼�

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // draw objects
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // set light uniforms
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        shader.setInt("blinn", blinn);//�л�phong��blinn-phongģʽ
        // floor
        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        std::cout << (blinn ? "Blinn-Phong" : "Phong") << std::endl;


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //��Ⱦѭ�����ͷ�������Դ
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);

    glfwTerminate();
    return 0;
}
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    stbi_set_flip_vertically_on_load(false);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}