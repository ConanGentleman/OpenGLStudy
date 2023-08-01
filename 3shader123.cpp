#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"
using namespace std;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor; // 颜色变量的属性位置值为 1\n"
"out vec3 ourColor; // 向片段着色器输出一个颜色\n"
"out vec3 ourPos;\n"
"uniform float posoff;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x+posoff,aPos.y,aPos.z, 1.0);//gl_Position是OpenGL的标准化坐标所以是vec4\n"
"   //ourColor = aColor; // 将ourColor设置为我们从顶点数据那里得到的输入颜色\n"
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
        // 位置              // 颜色
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // 右下
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // 左下
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // 顶部
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

    // 0. 复制顶点数组到缓冲中供OpenGL使用
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);


    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//告诉OpenGL该如何解析顶点数据（应用到逐个顶点属性上）,3 * sizeof(float)也可以设置为0来让OpenGL决定具体步长是多少（只有当数值是紧密排列时才可用）
    glEnableVertexAttribArray(0);                                              
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();//程序对象
    glAttachShader(shaderProgram, vertexShader);//将编译的顶点着色器附加到程序对象
    glAttachShader(shaderProgram, fragmentShader);//将编译的片元着色器附加到程序对象
    glLinkProgram(shaderProgram);//把它们链接(Link)为一个着色器程序对象
    // 3. 当我们渲染一个物体时要使用着色器程序
    //glUseProgram(shaderProgram1);//激活程序对象

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


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
        glClear(GL_COLOR_BUFFER_BIT);//颜色清空屏幕

        glUseProgram(shaderProgram);//激活程序对象
        int posoffLocation = glGetUniformLocation(shaderProgram, "posoff");
        //cout << posoffLocation << endl;
        if (posoffLocation!=-1) {
            //glUniform1f
            glUniform1f(posoffLocation, 0.5);
        }
        //// 更新uniform颜色
        //float timeValue = glfwGetTime();
        //float greenValue = sin(timeValue) / 2.0f + 0.5f;
        //int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        //if(vertexColorLocation!=-1) //找不到为-1
        //    glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);


        glBindVertexArray(VAO);//防止对于每一个顶点数据缓冲VBO处理一次效率不高，使用VAO一起处理
        glDrawArrays(GL_TRIANGLES, 0, 3);//使用VAO绘制三角形,参数为：图元类型，起始顶点索引和画多少个
        //glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);//使用EBO绘制三角形,参数为：图元类型，画多少个,索引类型，偏移量
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 可选:一旦资源超出其用途，就取消分配所有资源:
// ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glDeleteProgram(shaderProgram);
    //渲染循环后释放所有资源
    glfwTerminate();
    return 0;
}