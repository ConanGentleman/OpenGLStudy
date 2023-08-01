#define STB_IMAGE_IMPLEMENTATION
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "stb_image.h"
#include<iostream>
using namespace std;
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor; // 颜色变量的属性位置值为 1\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"out vec3 ourColor; // 向片段着色器输出一个颜色\n"
"out vec3 ourPos;\n"
"uniform mat4 transform;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n "
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);//gl_Position是OpenGL的标准化坐标所以是vec4\n"
"   //ourColor = aColor; // 将ourColor设置为我们从顶点数据那里得到的输入颜色\n"
"   ourPos=aColor;\n"
"   TexCoord = aTexCoord;\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"in vec2 TexCoord;\n"
"in vec3 ourColor;\n"
"out vec4 FragColor;\n"
"uniform sampler2D ourTexture;\n"
"uniform sampler2D ourTexture1;\n"
"uniform float mixValue;\n"
"void main()\n"
"{\n"
"   FragColor = mix(texture(ourTexture, TexCoord), texture(ourTexture1, vec2(TexCoord.x, TexCoord.y)), mixValue);\n"
"}\n\0";
const char* fragmentShader1Source = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 ourColor; // 在OpenGL程序代码中设定这个变量\n"
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
float mixValue = 0.2f;
float screenWidth = 800;
float screenHeight = 600;
float fov = 45.0f;
float aspect = 800.0f / 600.0f;
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间
float lastX = 400, lastY = 300;//鼠标的坐标
bool firstMouse = true;
float yaw = -90.0f;	// 偏航被初始化为-90.0度，因为0.0的偏航导致一个指向右的方向矢量，所以我们最初向左旋转一点。
float pitch = 0.0f;//俯仰角
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);//屏幕缩放事件
void mouse_callback(GLFWwindow* window, double xpos, double ypos);//鼠标移动事件
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//滚轮事件
void processInput(GLFWwindow* window);//按键输入事件
void mouse_callback(GLFWwindow* window, double xpos, double ypos)//鼠标回调事件
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if(fov >= 1.0f && fov <= 45.0f)
    fov -= yoffset;
  if(fov <= 1.0f)
    fov = 1.0f;
  if(fov >= 45.0f)
    fov = 45.0f;
}
/*
使用GLFW的glfwGetKey函数，它需要一个窗口以及一个按键作为输入。
这个函数将会返回这个按键是否正在被按下。
我们将创建一个processInput函数来让所有的输入代码保持整洁
返回键(Esc)（如果没有按下，glfwGetKey将会返回GLFW_RELEASE。
如果用户的确按下了返回键，我们将通过glfwSetwindowShouldClose使用把WindowShouldClose属性设置为 true的方法关闭GLFW。
下一次while循环的条件检测将会失败，程序将会关闭。
*/
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mixValue += 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue >= 1.0f)
            mixValue = 1.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mixValue -= 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue <= 0.0f)
            mixValue = 0.0f;
    }
    //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //{
    //    fov -= 0.01f; // change this value accordingly (might be too slow or too fast based on system hardware)

    //}
    //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //{
    //    fov += 0.01f; // change this value accordingly (might be too slow or too fast based on system hardware)
    //}
    //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //{
    //    aspect -= 0.01f; // change this value accordingly (might be too slow or too fast based on system hardware)

    //}
    //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //{
    //    aspect += 0.01f; // change this value accordingly (might be too slow or too fast based on system hardware)
    //}
    //不同的处理器会以更高的频率调用processInput函数，这会导致在不同的处理器上移动的速度不同，因此通过时间差来控制移动速度，


    float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)//使用叉乘来创建一个右向量(Right Vector)，并沿着它相应移动就可以了。这样就创建了使用摄像机时熟悉的横移(Strafe)效果
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;//对右向量进行了标准化保证匀速，叉乘结果会根据cameraFront变量返回大小不同的向量。如果我们不对向量进行标准化，我们就得根据摄像机的朝向不同加速或减速移动
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    cameraPos.y = 0;
}
//当用户改变窗口的大小的时候，视口也应该被调整。我们可以对窗口注册一个回调函数(Callback Function)，
//它会在每次窗口大小被调整的时候被调用。这个回调函数的原型如下
//需要一个GLFWwindow作为它的第一个参数，以及两个整数表示窗口的新维度。每当窗口改变大小，
//GLFW会调用这个函数并填充相应的参数供你处理。
//还需要注册这个函数，告诉GLFW我们希望每当窗口调整大小的时候调用这个函数：
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
glm::mat4 trans;
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

void transform_rotationOP() {
    trans = glm::mat4(1.0f);
    //trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));//旋转90度，绕z轴
    //trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));//每个轴缩放到0.5倍；

    //先位移再旋转
    trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));//移动到右下角
    trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));//随着时间旋转

    ////先旋转再位移
    //trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));//随着时间旋转
    //trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));//移动到右下角
    //cout << trans.length() << endl;
}
void scaleOP() {
    trans = glm::mat4(1.0f);
    //trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));//旋转90度，绕z轴
    //trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));//每个轴缩放到0.5倍；
    trans = glm::translate(trans, glm::vec3(-0.5f, 0.5f, 0.0f));
    float scaleAmount = static_cast<float>(sin(glfwGetTime()));
    //随时间缩放
    trans = glm::scale(trans, glm::vec3(scaleAmount, scaleAmount, scaleAmount));
    ////先旋转再位移
    //trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));//随着时间旋转
    //trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));//移动到右下角
    //cout << trans.length() << endl;
}
void modelmat() {
    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));//将其绕着x轴旋转，使它看起来像放在地上一样
    //model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
}
void viewmat() {
    view = glm::mat4(1.0f);
    // 注意，我们将矩阵向我们要进行移动场景的反方向移动。
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
}
void projectionmat() {
    projection = glm::mat4(1.0f);
    //projection = glm::perspective(glm::radians(45.0f), screenWidth / screenHeight, 0.1f, 100.0f);//第一个参数定义了fov的值，它表示的是视野(Field of View),通常设置为45.0f.第二个参数设置了宽高比，由视口的宽除以高所得。第三和第四个参数设置了平截头体的近和远平面
    projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);//第一个参数定义了fov的值，它表示的是视野(Field of View),通常设置为45.0f.第二个参数设置了宽高比，由视口的宽除以高所得。第三和第四个参数设置了平截头体的近和远平面
}
//LookAt函数
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
    glfwInit();//初始化glfw
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint函数的第一个参数代表选项的名称，我们可以从很多以GLFW_开头的枚举值中选择；
    //第二个参数接受一个整型，用来设置这个选项的值
    // GLFW_CONTEXT_VERSION_MAJOR表示所选客户端 API 的任何有效主版本号（OpenGL的版本为3.3 所以主版本号 为3）
    //该函数的所有的选项以及对应的值都可以在 GLFW’s window handling(https://www.glfw.org/docs/latest/window.html#window_hints) 这篇文档中找到
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // GLFW_CONTEXT_VERSION_MINOR表示所选客户端 API 的任何有效次版本号（OpenGL的版本为3.3 所以次版本号 为3）
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //GLFW_OPENGL_PROFILE表示上下文使用的 OpenGL 配置文件,值为GLFW_OPENGL_CORE_PROFILE，即告诉GLFW我们使用的是核心模式(Core-profile)
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);//如果使用的是Mac OS X系统，你还需要加这行代码到你的初始化代码中这些配置才能起作用

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);//通知GLFW将我们窗口的上下文设置为当前线程的主上下文了
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);//注册回调函数
    glfwSetCursorPosCallback(window, mouse_callback);//注册回调函数
    glfwSetScrollCallback(window, scroll_callback);//注册回调函数
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//告诉GLFW捕获我们的鼠标
    //GLAD是用来管理OpenGL的函数指针的，
    //所以在调用任何OpenGL的函数之前我们需要初始化GLAD
    //给GLAD传入了用来加载系统相关的OpenGL函数指针地址的函数。GLFW给我们的是glfwGetProcAddress
    //，它根据我们编译的系统定义了正确的函数
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //OpenGL渲染窗口的尺寸大小，即视口(Viewport)
    //glViewport函数前两个参数控制窗口左下角的位置。第三个和第四个参数控制渲染窗口的宽度和高度
    //际上也可以将视口的维度设置为比GLFW的维度小，这样子之后所有的OpenGL渲染将会在一个更小的窗口中显示，
    //这样子的话我们也可以将一些其它元素显示在OpenGL视口之外
    //OpenGL坐标范围只为-1到1，将(-1到1)范围内的坐标映射到(0, 800)和(0, 600)
    glViewport(0, 0, 800, 600);

    //窗口被第一次显示的时候framebuffer_size_callback也会被调用。
    //对于视网膜(Retina)显示屏，width和height都会明显比原输入值更高一点
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    float vertices1[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    //十个立方体的位置
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
    //unsigned int indices[] = {
    //     0, 1, 3, // first triangle
    //     1, 2, 3  // second triangle
    //};
    glEnable(GL_DEPTH_TEST);//开启深度测试
#pragma region MyRegion
    // 2. 设置顶点属性指针
    unsigned int vertexShader;//(https://learnopengl-cn.github.io/01%20Getting%20started/04%20Hello%20Triangle/)
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);//编译着色器源码





    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)//判断着色器是否编译成功
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
#pragma endregion
    //读取纹理图片

    unsigned int texture, texture1;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);//它会自动把纹理赋值给片段着色器的采样器

    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//当进行放大(Magnify)和缩小(Minify)操作的时候可以设置纹理过滤的选项
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //设置纹理的旋转，在y轴上翻转加载的纹理
    stbi_set_flip_vertically_on_load(true);
    //加载并生成纹理
    int width, height, nrChannels;
    int width1, height1, nrChannels1;
    unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);

    if (data) {
        //cout << "1" << endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);//释放图像内存
    glGenTextures(1, &texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//当进行放大(Magnify)和缩小(Minify)操作的时候可以设置纹理过滤的选项
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned char* data1 = stbi_load("awesomeface.png", &width1, &height1, &nrChannels1, 0);
    //stbi_image_free(data);//释放图像内存
    if (data1) {
        cout << "1" << endl;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data1);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data1);//释放图像内存
    // 0. 复制顶点数组到缓冲中供OpenGL使用
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);


    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);//告诉OpenGL该如何解析顶点数据（应用到逐个顶点属性上）,3 * sizeof(float)也可以设置为0来让OpenGL决定具体步长是多少（只有当数值是紧密排列时才可用）
    glEnableVertexAttribArray(0);
    //// 颜色属性
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    // 纹理属性
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);


    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();//程序对象
    glAttachShader(shaderProgram, vertexShader);//将编译的顶点着色器附加到程序对象
    glAttachShader(shaderProgram, fragmentShader);//将编译的片元着色器附加到程序对象
    glLinkProgram(shaderProgram);//把它们链接(Link)为一个着色器程序对象
    // 3. 当我们渲染一个物体时要使用着色器程序
    //glUseProgram(shaderProgram1);//激活程序对象

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);//激活程序对象
    glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0); // 手动设置
    glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture1"), 1); // 手动设置

    //glBindBuffer(GL_ARRAY_BUFFER, 0);


    //设置为线框模式而不是填充，可用用glPolygonMode(GL_FRONT_AND_BACK, GL_FILL)将其设置回默认模式。或者直接注释下面一行的代码
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //你可以在之后解除对VAO的绑定，这样其他VAO调用就不会意外地修改这个VAO，但这种情况很少发生。修改其他
    // VAOs无论如何都需要调用glBindVertexArray，所以当不直接需要时，我们通常不会取消绑定VAOs(也不会取消绑定vbo)。
    glBindVertexArray(0);

    /*
    希望程序在我们主动关闭它之前不断绘制图像并能够接受用户输入。
    因此，我们需要在程序中添加一个while循环，我们可以把它称之为渲染循环(Render Loop)，
    它能在我们让GLFW退出前一直保持运行。下面几行的代码就实现了一个简单的渲染循环：

    glfwWindowShouldClose函数在我们每次循环的开始前检查一次GLFW是否被要求退出，
    如果是的话该函数返回true然后渲染循环便结束了，之后为我们就可以关闭应用程序了。
    glfwPollEvents函数检查有没有触发什么事件（比如键盘输入、鼠标移动等）、更新窗口状态，
    并调用对应的回调函数（可以通过回调方法手动设置）。
    glfwSwapBuffers函数会交换颜色缓冲（它是一个储存着GLFW窗口每一个像素颜色值的大缓冲），
    它在这一迭代中被用来绘制，并且将会作为输出显示在屏幕上。
    */
    while (!glfwWindowShouldClose(window))
    {
        //输入
        processInput(window);//循环时监控是否按下键
        //渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置屏幕颜色
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//颜色清空屏幕|清楚深度缓冲
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, texture);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture1);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glUseProgram(shaderProgram);//激活程序对象
        //int posoffLocation = glGetUniformLocation(shaderProgram, "posoff");
        ////cout << posoffLocation << endl;
        //if (posoffLocation != -1) {
        //    //glUniform1f
        //    glUniform1f(posoffLocation, 0.5);
        //}
        int mixValueLocation = glGetUniformLocation(shaderProgram, "mixValue");
        if (mixValueLocation != -1) {
            //cout << mixValue << endl;
            glUniform1f(mixValueLocation, mixValue);
        }
        modelmat();
        viewmat();
        projectionmat();
        int transformLoc = glGetUniformLocation(shaderProgram, "model");
        if (transformLoc != -1) {
            //cout << -1 << endl;
            //transform_rotationOP();
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));//位置，矩阵数量，是否转置，
        }
        transformLoc = glGetUniformLocation(shaderProgram, "view");
        if (transformLoc != -1) {
            //cout << -1 << endl;
            //transform_rotationOP();
            //相机绕中心旋转
            //float radius = 10.0f;
            //float camX = sin(glfwGetTime()) * radius;
            //float camZ = cos(glfwGetTime()) * radius;
            //view=glm::mat4(1.0f);
            //view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);//参数为相机位置，目标位置，Up方向所在位置
            
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));//位置，矩阵数量，是否转置，
        }
        transformLoc = glGetUniformLocation(shaderProgram, "projection");
        if (transformLoc != -1) {
            //cout << -1 << endl;
            //transform_rotationOP();
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));//位置，矩阵数量，是否转置，
        }
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            if (i % 3 == 0)  // every 3rd iteration (including the first) we set the angle using GLFW's time function.
                angle = glfwGetTime() * 25.0f;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            transformLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        //glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 可选:一旦资源超出其用途，就取消分配所有资源:
// ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    //渲染循环后释放所有资源
    glfwTerminate();
    return 0;
}
