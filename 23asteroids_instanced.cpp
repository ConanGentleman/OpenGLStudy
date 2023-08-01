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

    //stbi_set_flip_vertically_on_load(true);//纹理y轴翻转

    //OpenGL渲染窗口的尺寸大小，即视口(Viewport)
    //glViewport函数前两个参数控制窗口左下角的位置。第三个和第四个参数控制渲染窗口的宽度和高度
    //际上也可以将视口的维度设置为比GLFW的维度小，这样子之后所有的OpenGL渲染将会在一个更小的窗口中显示，
    //这样子的话我们也可以将一些其它元素显示在OpenGL视口之外
    //OpenGL坐标范围只为-1到1，将(-1到1)范围内的坐标映射到(0, 800)和(0, 600)
    glViewport(0, 0, 800, 600);

    //窗口被第一次显示的时候framebuffer_size_callback也会被调用。
    //对于视网膜(Retina)显示屏，width和height都会明显比原输入值更高一点
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 告诉stb_image.h在y轴上翻转加载的纹理(在加载模型之前)。
    stbi_set_flip_vertically_on_load(true);

    //配置全局OpenGL状态
    glEnable(GL_DEPTH_TEST);//开启深度测试
    //glCullFace(GL_BACK);
    //glFrontFace(GL_CW);
    //glDepthFunc(GL_ALWAYS);//总是通过深度测试，功能与glDisable(GL_DEPTH_TEST);一样

    //Shader shader("20.cubemaps.vs", "20.cubemaps.fs");
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader asteroidShader("23.asteroids.vs", "23.asteroids.fs");
    Shader planetShader("23.planet.vs", "23.planet.fs");

    // load models
    // -----------
    Model rock("rock/rock.obj");
    Model planet("planet/planet.obj");

    // 生成一个大的半随机模型转换矩阵列表
    // ------------------------------------------------------------------
    unsigned int amount = 100000; //小行星数量
    glm::mat4* modelMatrices; //小行星的模型变换矩阵 指针
    modelMatrices = new glm::mat4[amount];
    srand(static_cast<unsigned int>(glfwGetTime())); // 初始化随机数种子
    float radius = 150.0;//大行星半径
    float offset = 25.0f;//每个小行星的偏移值
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. 变换: 在[-offset, offset]的范围内，沿半径圆移动
        float angle = (float)i / (float)amount * 360.0f; //所在角度
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;//偏移情况
        float x = sin(angle) * radius + displacement;//最终x的位置
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;//y的位置
        float y = displacement * 0.4f; //保持小行星场的高度小于x和z的宽度
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;//深度
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2.缩放：在0.05和0.25f之间缩放
        float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
        model = glm::scale(model, glm::vec3(scale));

        // 3. 转：绕随机旋转轴旋转
        float rotAngle = static_cast<float>((rand() % 360));
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));//

        // 4. 添加到矩阵列表中
        modelMatrices[i] = model;
    }

    // 配置实例数组（配置vs读取的偏移信息）
    // -------------------------
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
    
    
    //将变换矩阵设置为实例顶点属性(除数为1)
    //注意:我们采取了一点欺骗，现在公开声明，模型的网格(es)的VAO和添加新的vertexAttribPointers
    //通常情况下，你会希望以一种更有组织的方式来做这件事，但为了学习的目的，这也可以。
    for (unsigned int i = 0; i < rock.meshes.size(); i++)
    {
        unsigned int VAO = rock.meshes[i].VAO;
        glBindVertexArray(VAO);
        // 顶点属性
        //这里的位置、颜色，法线都没用上（并且在model.h中已经提前将glVertexAttribPointer对0、1、2进行了定义），所以从3,4,5,6开始代表为一个的行
        //这里为什么要分4个呢，因为vs第三个输入参数是一个4*4矩阵，所以用4个vector4来替代
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1); //每渲染1个新实例的时候更新顶点属性
        glVertexAttribDivisor(4, 1);//每渲染1个新实例的时候更新顶点属性
        glVertexAttribDivisor(5, 1);//每渲染1个新实例的时候更新顶点属性
        glVertexAttribDivisor(6, 1);//每渲染1个新实例的时候更新顶点属性

        glBindVertexArray(0);
    }


    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 配置变换矩阵
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        asteroidShader.use();
        asteroidShader.setMat4("projection", projection);
        asteroidShader.setMat4("view", view);
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);

        // 画出大行星
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        planetShader.setMat4("model", model);
        planet.Draw(planetShader);

        // 画出陨石（小行星）
        asteroidShader.use();
        asteroidShader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rock.textures_loaded[0].id); // 注意:我们还将textures_loaded向量从model类中设置为public(而不是private)。
        for (unsigned int i = 0; i < rock.meshes.size(); i++)
        {
            glBindVertexArray(rock.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(rock.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount);//使用索引缓存 因此使用glDrawElementsInstanced绘制
            glBindVertexArray(0);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //渲染循环后释放所有资源

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


