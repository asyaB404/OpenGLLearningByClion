// ============================================================================
// Lesson 4.1: 纹理（Texture）
// ============================================================================
// 本课程学习内容：
// 1. 什么是纹理以及如何使用
// 2. 纹理坐标（Texture Coordinates）的概念
// 3. 如何将纹理应用到几何体上
// 4. 多个纹理的使用（纹理单元）
// 5. 纹理参数设置（过滤、环绕模式）
// 6. 使用 Shader 类管理着色器
// 7. 使用 stb_image 加载图片文件
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>
#include <string>  // 用于 std::string
#include "common/common.h"  // 公共工具函数（回调函数和输入处理）
#include "common/shader.h" // Shader 类

// ============================================================================
// 全局常量定义
// ============================================================================
static const unsigned int SCR_WIDTH = 800;
static const unsigned int SCR_HEIGHT = 600;

// ============================================================================
// 辅助函数：创建程序生成的纹理（用于演示，不需要加载图片）
// ============================================================================
// 使用 static 关键字，使函数只在当前文件内可见，避免链接错误
static unsigned int createProceduralTexture()
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
// 辅助函数：加载纹理（使用 stb_image）
// ============================================================================
// 参数：
//   - path: 图片文件路径
//   - flipVertically: 是否垂直翻转图片（OpenGL 的纹理坐标原点在左下角）
// 返回：纹理 ID
// 使用 static 关键字，使函数只在当前文件内可见，避免链接错误
// ============================================================================
static unsigned int loadTexture(const char* path, bool flipVertically = true)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    
    // 设置是否垂直翻转图片
    // OpenGL 的纹理坐标原点在左下角，而大多数图片格式的原点在左上角
    stbi_set_flip_vertically_on_load(flipVertically);
    
    // 加载图片
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data)
    {
        // 根据通道数确定格式
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // 释放图片数据
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        // 加载失败时，删除已创建的纹理对象并返回 0
        glDeleteTextures(1, &textureID);
        return 0;
    }
    
    return textureID;
}

// ============================================================================
// Lesson 4.1 主函数
// ============================================================================
int lesson4_1_main()
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
    // 第四步：创建和编译着色器程序（使用 Shader 类）
    // ========================================================================
    // Shader 类会自动从文件加载、编译和链接着色器
    // 使用 PROJECT_ROOT 宏（由 CMake 定义）来构建完整路径
    // ========================================================================
    std::string vertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson4/4.1.texture.vs";
    std::string fragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson4/4.1.texture.fs";
    Shader ourShader(vertexPath.c_str(), fragmentPath.c_str());

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
    // 第六步：加载纹理
    // ========================================================================
    // 使用 stb_image 加载真实图片文件
    // 纹理文件路径：engine/assets/texture/lesson/wall.jpg
    // 注意：路径相对于程序运行时的当前工作目录（通常是 cmake-build-debug）
    // ========================================================================
    unsigned int texture1, texture2;
    
    // 纹理 1：加载 wall.jpg 图片
    // 使用 PROJECT_ROOT 宏（由 CMake 定义）来构建完整路径
    // 如果文件不存在，loadTexture 会返回 0，我们可以使用程序生成的纹理作为备选
    std::string texturePath = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/wall.jpg";
    texture1 = loadTexture(texturePath.c_str());
    if (texture1 == 0) {
        std::cout << "警告：无法加载 wall.jpg，使用程序生成的纹理" << std::endl;
        texture1 = createProceduralTexture();
    }
    
    // 纹理 2：创建程序生成的纹理（用于混合效果）
    // 如果你想使用另一个图片，可以取消下面的注释：
    // texture2 = loadTexture("engine/assets/texture/lesson/another_texture.jpg");
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ========================================================================
    // 第七步：告诉 OpenGL 每个采样器对应哪个纹理单元
    // ========================================================================
    // 纹理单元（Texture Unit）是 OpenGL 中用于管理多个纹理的机制
    // GL_TEXTURE0 是默认激活的纹理单元
    // 使用 Shader 类的 setInt 方法设置 uniform 变量
    // ========================================================================
    ourShader.use();  // 激活着色器程序
    ourShader.setInt("texture1", 0);  // 纹理单元 0 对应 texture1
    ourShader.setInt("texture2", 1);  // 纹理单元 1 对应 texture2

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
        ourShader.use();  // 使用 Shader 类的 use 方法
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
    // Shader 类的析构函数会自动删除着色器程序，但我们可以显式调用
    // 实际上，Shader 类没有提供删除方法，程序结束时会自动清理

    glfwTerminate();
    return 0;
}

