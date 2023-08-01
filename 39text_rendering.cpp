#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <map>
#include <string>
using namespace std;




void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

/// ����ʹ��FreeType����ʱ���ַ���ص�����״̬��Ϣ
struct Character {
    unsigned int TextureID; // ���������ID
    glm::ivec2   Size;      // ���δ�С
    glm::ivec2   Bearing;  // �ӻ�׼�ߵ�������/������ƫ��ֵ
    unsigned int Advance;  // ԭ�����һ������ԭ��ľ���
};

std::map<GLchar, Character> Characters;
unsigned int VAO, VBO;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    //Ƭ����ɫ��������uniform������һ���ǵ���ɫͨ��������λͼ������һ������ɫuniform�����������������ı���������ɫ��
    //�������ȴ�λͼ�����в�����ɫֵ���������������н��洢�ź�ɫ���������ǾͲ��������r��������Ϊȡ����alphaֵ��
    //ͨ���任��ɫ��alphaֵ�����յ���ɫ�����α�����ɫ�ϻ���͸���ģ������������ַ��������ǲ�͸���ġ�����Ҳ��RGB��ɫ��textColor���uniform��ˣ����任�ı���ɫ��
    //��Ȼ������Ҫ���û�ϲ�������һ����֮��Ч��
    glEnable(GL_BLEND);//���α�����ɫ��͸��
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // compile and setup the shader
    // ----------------------------
    Shader shader("39.text.vs", "39.text.fs");
    //����ͶӰ�������ǽ�ʹ��һ������ͶӰ����(Orthographic Projection Matrix)�������ı���Ⱦ���ǣ�ͨ����������Ҫ͸�ӣ�
    //ʹ������ͶӰͬ��������������Ļ����ϵ���趨���еĶ������꣬�������ʹ�����·�ʽ���ã�
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    //Ҫ����һ�����壬����ֻ��Ҫ��ʼ��FreeType�⣬���ҽ�����������Ϊһ��FreeType��֮Ϊ��(Face)�Ķ�����
    //����Ϊ���Ǽ���һ����Windows/FontsĿ¼�п�������TrueType�����ļ�arial.ttf��
    FT_Library ft;
    // ����������ʱ�����к���������һ��������0��ֵ
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    //�ҵ�font��·��
    std::string font_name = "fonts/Antonio-Bold.ttf";
    if (font_name.empty())
    {
        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
        return -1;
    }

    //����������Ϊ��
    FT_Face face;
    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        //Ϊ�����������ô�С
        FT_Set_Pixel_Sizes(face, 0, 48);

        //�����ֽڶ�������
        //OpenGLҪ�����е�������4�ֽڶ���ģ�������Ĵ�С��Զ��4�ֽڵı�����
        //ͨ���Ⲣ�������ʲô���⣬��Ϊ�󲿷�����Ŀ�ȶ�Ϊ4�ı�����/��ÿ����ʹ��4���ֽڣ�������������ÿ������ֻ����һ���ֽڣ�������������Ŀ�ȡ�
        //ͨ���������ֽڶ��������Ϊ1����������ȷ�������ж������⣨�����ܻ���ɶδ���
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        //�����forѭ�������Ǳ�����ASCII���е�ȫ��128���ַ�������ȡ���Ƕ�Ӧ���ַ����Ρ�
        //��ÿһ���ַ�������������һ����������������ѡ������������Ķ���ֵ��
        //��Ȥ�����������ｫ�����internalFormat��format����ΪGL_RED��ͨ���������ɵ�λͼ��һ��8λ�Ҷ�ͼ������ÿһ����ɫ����һ���ֽ�����ʾ��
        //���������Ҫ��λͼ�����ÿһ�ֽڶ���Ϊ�������ɫֵ������ͨ������һ�����������ʵ�ֵģ���������ÿһ�ֽڶ���Ӧ��������ɫ�ĺ�ɫ��������ɫ�����ĵ�һ���ֽڣ���
        //�������ʹ��һ���ֽ�����ʾ�������ɫ��������Ҫע��OpenGL��һ�����ƣ������ֽڶ������ƣ�����һ�У�
        for (unsigned char c = 0; c < 128; c++)
        {
            // �����ַ�����
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // ��������
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // ��������ѡ��
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // ���ڴ洢�ַ��Ա��Ժ�ʹ��
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // ���㴦�������κ�Ҫ��������FreeType����Դ��
    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    //Ϊ�����ı�������VAO/VBO
    // -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        RenderText(shader, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
        RenderText(shader, "(C) LearnOpenGL.com", 540.0f, 570.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f));

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// ��Ⱦ�ı���
// -------------------
//�������������Ӧ�÷ǳ������ˣ��������ȼ�����ı��ε�ԭ�����꣨Ϊxpos��ypos�������Ĵ�С��Ϊw��h����������6�������γ����2D�ı��Σ�ע�����ǽ�ÿ������ֵ��ʹ��scale�������š�
//���������Ǹ�����VBO�����ݡ�����Ⱦ������ı��Ρ�
void RenderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color)
{
    //������Ӧ����Ⱦ״̬
    shader.use();
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // �����ı������е��ַ�
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        //һЩ�ַ����硯p����q������Ҫ����Ⱦ����׼�����£���������ı���ҲӦ�ñ��ڷ���RenderText��yֵ���¡�
        //ypos��ƫ�������Դ����εĶ���ֵ�еó���
        //Ҫ������ξ��룬��ƫ������������Ҫ�ҳ������ڻ�׼��֮����չ��ȥ�ľ��롣����ͼ����ξ����ú�ɫ��ͷ�����
        //�Ӷ���ֵ�п��Կ��������ǿ���ͨ�������εĸ߶ȼ�ȥbearingY��������������ĳ��ȡ�������Щ����λ�ڻ�׼���ϵ��ַ����硯X���������ֵ������0.0��
        //��������Щ������׼�ߵ��ַ����硯g����j���������ֵ�������ġ�
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // ��ÿ���ַ�����VBO
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // ���ı����ϻ�����������
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // ����VBO�ڴ������
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // �����ı���
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // ����λ�õ���һ�����ε�ԭ�㣬ע�ⵥλ��1/64����
        x += (ch.Advance >> 6) * scale; // λƫ��6����λ����ȡ��λΪ���ص�ֵ (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}