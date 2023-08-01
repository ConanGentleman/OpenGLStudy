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
"   FragPos = vec3(model * vec4(aPos, 1.0)); //片段在世界坐标的位置，以便于与光照求光照方向\n"
"   Normal = mat3(transpose(inverse(model))) * aNormal; //法线的位置通过模型矩阵的左上角3*3部分的逆矩阵的转置矩阵来变换为世界坐标下的位置\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);//gl_Position是OpenGL的标准化坐标所以是vec4\n"
"   TexCoords = aTexCoords;\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"struct Material {//移除了环境光材质颜色向量，因为环境光颜色在几乎所有情况下都等于漫反射颜色，所以我们不需要将它们分开储存：\n"
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

"   //点光源衰减设置\n"
"   float constant;\n"
"   float linear;\n"
"   float quadratic;\n"
"}; \n"
"in vec2 TexCoords;\n"
"in vec3 Normal;\n"
"in vec3 FragPos;\n"

"out vec4 FragColor;\n"

"uniform vec3 viewPos;//视角所在位置（相机位置）\n"
"uniform Material material;\n"
"uniform Light light;\n"

"void main()\n"
"{\n"
"// 环境光照\n"
"   vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;//将环境光的材质颜色设置为漫反射材质颜色同样的值。\n"

"// 漫反射光照 \n"
"vec3 norm = normalize(Normal);//将法线归一化 \n"
"vec3 lightDir = normalize(light.position - FragPos); //光照方向\n"
"float diff = max(dot(norm, lightDir), 0.0);//求光照与法线的夹角cos \n"
"vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;//从纹理中采样片段的漫反射颜色值,得到漫反射项的光照 \n"

"// specular\n"
"vec3 viewDir = normalize(viewPos - FragPos);//视角方向 \n"
"vec3 reflectDir = reflect(-lightDir, norm);//求光线经过法线后的反射方向 \n"
"float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);//视角与反射方向的cos \n"
" vec3 specular = light.specular * spec * texture(material.specular, TexCoords).rgb; //镜面反射分量 \n"

"// 聚光灯 (软边缘)\n"
"float theta = dot(lightDir, normalize(-light.direction));\n"
"float epsilon = (light.cutOff - light.outerCutOff);\n"
"float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);//clamp函数把第一个参数约束在了0.0到1.0之间 \n"
"diffuse *= intensity; \n"
"specular *= intensity;// 将不对环境光做出影响，让它总是能有一点光 \n"

"//衰减\n"
"   float distance    = length(light.position - FragPos);//计算光与片段的举例，length是自带的函数\n"
"   float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); //公式计算\n"
"   ambient *= attenuation;\n"
"   diffuse *= attenuation;\n"
"   specular *= attenuation;\n"

"//结果\n"
"vec3 result = ambient + diffuse + specular;//将环境光照与漫反射项体现在物体上 \n"
"FragColor = vec4(result, 1.0); \n"
"}\n\0";
const char* vertexShaderSourceLightcube = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n "
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);//gl_Position是OpenGL的标准化坐标所以是vec4\n"
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
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间
bool firstMouse = true;
float yaw = -90.0f;	// 偏航被初始化为-90.0度，因为0.0的偏航导致一个指向右的方向矢量，所以我们最初向左旋转一点。
float pitch = 0.0f;//俯仰角

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;//鼠标起始的坐标

//纹理贴图读取
unsigned int loadTexture(const char* path);
// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
//glm::vec3 lightDirection(0.2f, 1.0f, 0.3f);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);//屏幕缩放事件
void mouse_callback(GLFWwindow* window, double xpos, double ypos);//鼠标移动事件
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);//滚轮事件
void processInput(GLFWwindow* window);//按键输入事件
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

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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
void setLight() {
    model = glm::mat4();
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
}
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
    //十个箱子的位置
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

    unsigned int vertexShaderLightcube;//(https://learnopengl-cn.github.io/01%20Getting%20started/04%20Hello%20Triangle/)
    vertexShaderLightcube = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderLightcube, 1, &vertexShaderSourceLightcube, NULL);
    glCompileShader(vertexShaderLightcube);//编译着色器源码

    glGetShaderiv(vertexShaderLightcube, GL_COMPILE_STATUS, &success);
    if (!success)//判断着色器是否编译成功
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


    // 0. 复制顶点数组到缓冲中供OpenGL使用
    unsigned int VBO, cubeVAO;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //顶点法线属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //// 颜色属性
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    //glEnableVertexAttribArray(1);
    // 纹理属性
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // 我们只需要绑定到VBO(链接它与glVertexAttribPointer)，不需要填充它;VBO的数据已经包含了我们需要的所有内容(它已经绑定了，但出于教育目的，我们再次这样做)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();//程序对象
    glAttachShader(shaderProgram, vertexShader);//将编译的顶点着色器附加到程序对象
    glAttachShader(shaderProgram, fragmentShader);//将编译的片元着色器附加到程序对象
    glLinkProgram(shaderProgram);//把它们链接(Link)为一个着色器程序对象
    // 3. 当我们渲染一个物体时要使用着色器程序
    //glUseProgram(shaderProgram1);//激活程序对象

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    unsigned int shaderProgramLightcube;
    shaderProgramLightcube = glCreateProgram();//程序对象
    glAttachShader(shaderProgramLightcube, vertexShaderLightcube);//将编译的顶点着色器附加到程序对象
    glAttachShader(shaderProgramLightcube, fragmentShaderLightcube);//将编译的片元着色器附加到程序对象
    glLinkProgram(shaderProgramLightcube);//把它们链接(Link)为一个着色器程序对象
    // 3. 当我们渲染一个物体时要使用着色器程序
    //glUseProgram(shaderProgram1);//激活程序对象

    glDeleteShader(vertexShaderLightcube);
    glDeleteShader(fragmentShaderLightcube);

    glUseProgram(shaderProgramLightcube);//激活程序对象

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
    string diffuseMappath = "container2.png";
    string specularMappath = "container2_specular.png";
    //string numberMappath = "matrix.jpg";
    unsigned int diffuseMap = loadTexture(diffuseMappath.c_str());
    unsigned int specularMap = loadTexture(specularMappath.c_str());
    //unsigned int numberMap = loadTexture(numberMappath.c_str());

    glUseProgram(shaderProgram);
    int transLoc = glGetUniformLocation(shaderProgram, "material.diffuse");
    glUniform1i(transLoc, 0);//如果纹理是GL_TEXTURE0，就给sampler2D赋值为0
    transLoc = glGetUniformLocation(shaderProgram, "material.specular");
    //glUniform3f(transformLoc, 0.5f, 0.5f, 0.5f);
    glUniform1i(transLoc, 1);
    while (!glfwWindowShouldClose(window))
    {

        //输入
        processInput(window);//循环时监控是否按下键
        //渲染
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);//设置屏幕颜色
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//颜色清空屏幕|清楚深度缓冲
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, texture);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture1);
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glUseProgram(shaderProgram);//激活程序对象

        ////让光照来回移动
        //lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
        //lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;
        int transformLoc;
        glm::vec3 lightColor;
        //transformLoc = glGetUniformLocation(shaderProgram, "lightColor");
        //glUniform3f(transformLoc, 1.0f, 1.0f, 1.0f);
        transformLoc = glGetUniformLocation(shaderProgram, "light.position");
        glUniform3f(transformLoc, camera.Position.x, camera.Position.y, camera.Position.z);//聚光灯位置就是视角
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
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // 减少影响
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // 很低的影响
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
        // 绑定漫反射贴图
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        //绑定镜面反射贴图
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        ////绑定数字放射光贴图
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, numberMap);

        glBindVertexArray(cubeVAO);
        //绘制十个箱子
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



        //绘制光源处盒子
        glUseProgram(shaderProgramLightcube);//激活程序对象
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
    // 可选:一旦资源超出其用途，就取消分配所有资源:
// ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderProgramLightcube);
    //渲染循环后释放所有资源
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//当进行放大(Magnify)和缩小(Minify)操作的时候可以设置纹理过滤的选项
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);//释放图像内存
    return textureID;
}