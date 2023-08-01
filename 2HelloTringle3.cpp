#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
using namespace std;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char* fragmentShader1Source = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
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


    unsigned int fragmentShader1,fragmentShader2;
    fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER);
    fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader1, 1, &fragmentShader1Source, NULL);
    glShaderSource(fragmentShader2, 1, &fragmentShader2Source, NULL);
    glCompileShader(fragmentShader1);
    glCompileShader(fragmentShader2);

    glGetShaderiv(fragmentShader1, GL_COMPILE_STATUS, &success);
    
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader1, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glGetShaderiv(fragmentShader2, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader2, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // 0. 复制顶点数组到缓冲中供OpenGL使用
    unsigned int VAOs[2], VBOs[2];
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);

    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(firstTriangle), firstTriangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);//告诉OpenGL该如何解析顶点数据（应用到逐个顶点属性上）
    glEnableVertexAttribArray(0);//以顶点属性位置值作为参数，启用顶点属性.默认禁用。

    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(secondTriangle), secondTriangle, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);//告诉OpenGL该如何解析顶点数据（应用到逐个顶点属性上）,3 * sizeof(float)也可以设置为0来让OpenGL决定具体步长是多少（只有当数值是紧密排列时才可用）
    glEnableVertexAttribArray(0);//以顶点属性位置值作为参数，启用顶点属性.默认禁用。

    unsigned int shaderProgram1;
    shaderProgram1 = glCreateProgram();//程序对象
    glAttachShader(shaderProgram1, vertexShader);//将编译的顶点着色器附加到程序对象
    glAttachShader(shaderProgram1, fragmentShader1);//将编译的片元着色器附加到程序对象
    glLinkProgram(shaderProgram1);//把它们链接(Link)为一个着色器程序对象
    // 3. 当我们渲染一个物体时要使用着色器程序
    //glUseProgram(shaderProgram1);//激活程序对象
    unsigned int shaderProgram2;
    shaderProgram2 = glCreateProgram();//程序对象
    glAttachShader(shaderProgram2, vertexShader);//将编译的顶点着色器附加到程序对象
    glAttachShader(shaderProgram2, fragmentShader2);//将编译的片元着色器附加到程序对象
    glLinkProgram(shaderProgram2);//把它们链接(Link)为一个着色器程序对象
    // 3. 当我们渲染一个物体时要使用着色器程序
    //glUseProgram(shaderProgram2);//激活程序对象

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader1);
    glDeleteShader(fragmentShader2);//原来的着色器就可以删除了


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

        glUseProgram(shaderProgram1);//激活程序对象

        glBindVertexArray(VAOs[0]);//防止对于每一个顶点数据缓冲VBO处理一次效率不高，使用VAO一起处理
        glDrawArrays(GL_TRIANGLES, 0, 3);//使用VAO绘制三角形,参数为：图元类型，起始顶点索引和画多少个
        //glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);//使用EBO绘制三角形,参数为：图元类型，画多少个,索引类型，偏移量
        glUseProgram(shaderProgram2);//激活程序对象
        glBindVertexArray(VAOs[1]);//防止对于每一个顶点数据缓冲VBO处理一次效率不高，使用VAO一起处理
        glDrawArrays(GL_TRIANGLES, 0, 3);//使用VAO绘制三角形,参数为：图元类型，起始顶点索引和画多少个
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 可选:一旦资源超出其用途，就取消分配所有资源:
// ------------------------------------------------------------------------
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);

    glDeleteProgram(shaderProgram1);
    glDeleteProgram(shaderProgram2);
    //渲染循环后释放所有资源
    glfwTerminate();
    return 0;
}