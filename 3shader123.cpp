#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"
using namespace std;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor; // ��ɫ����������λ��ֵΪ 1\n"
"out vec3 ourColor; // ��Ƭ����ɫ�����һ����ɫ\n"
"out vec3 ourPos;\n"
"uniform float posoff;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x+posoff,aPos.y,aPos.z, 1.0);//gl_Position��OpenGL�ı�׼������������vec4\n"
"   //ourColor = aColor; // ��ourColor����Ϊ���ǴӶ�����������õ���������ɫ\n"
"   ourPos=aPos;\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourPos;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(ourPos, 1.0f);\n"
"}\n\0";
const char* fragmentShader1Source = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 ourColor; // ��OpenGL����������趨�������\n"
"void main()\n"
"{\n"
"   FragColor = ourColor;\n"
"}\n\0";
const char* fragmentShader2Source = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
"}\n\0";

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

    float firstTriangle[] = {
        -0.9f, -0.5f, 0.0f,  // left 
        -0.0f, -0.5f, 0.0f,  // right
        -0.45f, 0.5f, 0.0f,  // top 
    };
    float secondTriangle[] = {
        0.0f, -0.5f, 0.0f,  // left
        0.9f, -0.5f, 0.0f,  // right
        0.45f, 0.5f, 0.0f   // top 
    };

    float vertices[] = {
        // λ��              // ��ɫ
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // ����
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // ����
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // ����
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

    // 0. ���ƶ������鵽�����й�OpenGLʹ��
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);


    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // λ������
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//����OpenGL����ν����������ݣ�Ӧ�õ�������������ϣ�,3 * sizeof(float)Ҳ��������Ϊ0����OpenGL�������岽���Ƕ��٣�ֻ�е���ֵ�ǽ�������ʱ�ſ��ã�
    glEnableVertexAttribArray(0);                                              
    // ��ɫ����
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();//�������
    glAttachShader(shaderProgram, vertexShader);//������Ķ�����ɫ�����ӵ��������
    glAttachShader(shaderProgram, fragmentShader);//�������ƬԪ��ɫ�����ӵ��������
    glLinkProgram(shaderProgram);//����������(Link)Ϊһ����ɫ���������
    // 3. ��������Ⱦһ������ʱҪʹ����ɫ������
    //glUseProgram(shaderProgram1);//����������

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


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
    while (!glfwWindowShouldClose(window))
    {
        //����
        processInput(window);//ѭ��ʱ����Ƿ��¼�
        //��Ⱦ
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//������Ļ��ɫ
        glClear(GL_COLOR_BUFFER_BIT);//��ɫ�����Ļ

        glUseProgram(shaderProgram);//����������
        int posoffLocation = glGetUniformLocation(shaderProgram, "posoff");
        //cout << posoffLocation << endl;
        if (posoffLocation!=-1) {
            //glUniform1f
            glUniform1f(posoffLocation, 0.5);
        }
        //// ����uniform��ɫ
        //float timeValue = glfwGetTime();
        //float greenValue = sin(timeValue) / 2.0f + 0.5f;
        //int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        //if(vertexColorLocation!=-1) //�Ҳ���Ϊ-1
        //    glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);


        glBindVertexArray(VAO);//��ֹ����ÿһ���������ݻ���VBO����һ��Ч�ʲ��ߣ�ʹ��VAOһ����
        glDrawArrays(GL_TRIANGLES, 0, 3);//ʹ��VAO����������,����Ϊ��ͼԪ���ͣ���ʼ���������ͻ����ٸ�
        //glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);//ʹ��EBO����������,����Ϊ��ͼԪ���ͣ������ٸ�,�������ͣ�ƫ����
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // ��ѡ:һ����Դ��������;����ȡ������������Դ:
// ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glDeleteProgram(shaderProgram);
    //��Ⱦѭ�����ͷ�������Դ
    glfwTerminate();
    return 0;
}