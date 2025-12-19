// ============================================================================
// Lesson 4: 纹理（Texture）
// ============================================================================
// 本课程学习内容：
// 1. 什么是纹理以及如何使用
// 2. 纹理坐标（Texture Coordinates）的概念
// 3. 如何将纹理应用到几何体上
// 4. 多个纹理的使用（纹理单元）
// 5. 纹理参数设置（过滤、环绕模式）
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "common/common.h"  // 公共工具函数（回调函数和输入处理）

// ============================================================================
// 全局常量定义
// ============================================================================
static const unsigned int SCR_WIDTH = 800;
static const unsigned int SCR_HEIGHT = 600;

// ============================================================================
// 顶点着色器源码（GLSL 语言）
// ============================================================================
// 这个着色器接收三个属性：
// 1. aPos (位置) - location = 0
// 2. aColor (颜色) - location = 1
// 3. aTexCoord (纹理坐标) - location = 2
// 
// 顶点着色器将纹理坐标传递给片段着色器
// ============================================================================
static const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"        // 输入：顶点位置
    "layout (location = 1) in vec3 aColor;\n"      // 输入：顶点颜色
    "layout (location = 2) in vec2 aTexCoord;\n"   // 输入：纹理坐标
    "out vec3 ourColor;\n"                          // 输出：传递给片段着色器的颜色
    "out vec2 TexCoord;\n"                          // 输出：传递给片段着色器的纹理坐标
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"          // 设置顶点位置
    "   ourColor = aColor;\n"                       // 将颜色传递给片段着色器
    "   TexCoord = aTexCoord;\n"                    // 将纹理坐标传递给片段着色器
    "}\0";

// ============================================================================
// 片段着色器源码（GLSL 语言）
// ============================================================================
// 这个着色器接收两个纹理（texture1 和 texture2）
// 并将它们混合在一起
// ============================================================================
static const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"                         // 输出：最终片段颜色
    "in vec3 ourColor;\n"                           // 输入：从顶点着色器传来的颜色
    "in vec2 TexCoord;\n"                           // 输入：从顶点着色器传来的纹理坐标
    "uniform sampler2D texture1;\n"                  // 纹理采样器 1
    "uniform sampler2D texture2;\n"                  // 纹理采样器 2
    "void main()\n"
    "{\n"
    "   // 混合两个纹理：texture1 占 80%，texture2 占 20%\n"
    "   FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);\n"
    "}\n\0";

// ============================================================================
// 辅助函数：创建程序生成的纹理（用于演示，不需要加载图片）
// ============================================================================
unsigned int createProceduralTexture()
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // 创建一个简单的 2x2 纹理（棋盘格图案）
    unsigned char data[] = {
        255, 0, 0,     // 红色
        0, 255, 0,     // 绿色
        0, 0, 255,     // 蓝色
        255, 255, 0   // 黄色
    };
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    return texture;
}

// ============================================================================
// 辅助函数：编译着色器
// ============================================================================
unsigned int compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // 检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    return shader;
}

// ============================================================================
// 辅助函数：创建着色器程序
// ============================================================================
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // 检查链接错误
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

// ============================================================================
// Lesson 4 主函数
// ============================================================================
int lesson4_main()
{
    // ========================================================================
    // 第一步：初始化 GLFW
    // ========================================================================
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // ========================================================================
    // 第二步：创建窗口
    // ========================================================================
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Learning - Lesson 4: Texture", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ========================================================================
    // 第三步：初始化 GLAD
    // ========================================================================
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ========================================================================
    // 第四步：创建和编译着色器程序
    // ========================================================================
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // ========================================================================
    // 第五步：设置顶点数据
    // ========================================================================
    // 顶点数据格式：位置(3) + 颜色(3) + 纹理坐标(2) = 8 个 float
    // 每个顶点有 8 个 float，总共 32 字节
    // ========================================================================
    float vertices[] = {
        // 位置 (x, y, z)        // 颜色 (R, G, B)        // 纹理坐标 (u, v)
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // 右上角
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // 右下角
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 左下角
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // 左上角
    };
    
    unsigned int indices[] = {
        0, 1, 3, // 第一个三角形：右上角 -> 右下角 -> 左上角
        1, 2, 3  // 第二个三角形：右下角 -> 左下角 -> 左上角
    };
    
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 位置属性（location = 0）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 颜色属性（location = 1）
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // 纹理坐标属性（location = 2）
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // ========================================================================
    // 第六步：创建纹理
    // ========================================================================
    // 创建两个程序生成的纹理（用于演示）
    // 在实际应用中，你可以使用 stb_image 加载图片文件
    // ========================================================================
    unsigned int texture1, texture2;
    
    // 纹理 1
    texture1 = createProceduralTexture();
    
    // 纹理 2（创建另一个纹理，使用不同的颜色）
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    unsigned char data2[] = {
        255, 255, 255,   // 白色
        128, 128, 128,   // 灰色
        128, 128, 128,   // 灰色
        255, 255, 255   // 白色
    };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // ========================================================================
    // 第七步：告诉 OpenGL 每个采样器对应哪个纹理单元
    // ========================================================================
    // 纹理单元（Texture Unit）是 OpenGL 中用于管理多个纹理的机制
    // GL_TEXTURE0 是默认激活的纹理单元
    // ========================================================================
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);  // 纹理单元 0
    glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);  // 纹理单元 1

    // ========================================================================
    // 第八步：渲染循环
    // ========================================================================
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入
        processInput(window);

        // 渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 绑定纹理到对应的纹理单元
        glActiveTexture(GL_TEXTURE0);  // 激活纹理单元 0
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);  // 激活纹理单元 1
        glBindTexture(GL_TEXTURE_2D, texture2);

        // 渲染容器
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // 交换缓冲区和轮询事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ========================================================================
    // 第九步：清理资源
    // ========================================================================
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

