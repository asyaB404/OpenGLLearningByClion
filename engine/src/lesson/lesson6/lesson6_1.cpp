// ============================================================================
// Lesson 6.1: 相机系统（Camera System）
// ============================================================================
// 本课程学习内容：
// 1. 理解相机系统的工作原理
// 2. 使用 Camera 类管理相机
// 3. 键盘控制相机移动（WASD）
// 4. 鼠标控制相机视角（鼠标移动）
// 5. 滚轮控制视野缩放（鼠标滚轮）
// 6. 时间差（deltaTime）用于平滑移动
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include "common/common.h"  // 公共工具函数（回调函数和输入处理）
#include "common/shader.h"  // Shader 类
#include "common/camera.h"  // Camera 类

// ============================================================================
// 全局常量定义
// ============================================================================
static const unsigned int SCR_WIDTH = 800;
static const unsigned int SCR_HEIGHT = 600;

// ============================================================================
// 全局变量
// ============================================================================
// 相机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 时间
float deltaTime = 0.0f;  // 当前帧和上一帧的时间差
float lastFrame = 0.0f;  // 上一帧的时间

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
// 鼠标移动回调函数
// ============================================================================
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
    float yoffset = lastY - ypos; // 注意：y 坐标是从下往上的，所以需要反转

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// ============================================================================
// 滚轮回调函数
// ============================================================================
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// ============================================================================
// 输入处理函数（重写，添加相机控制）
// ============================================================================
void processInputWithCamera(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 相机移动控制
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// ============================================================================
// Lesson 6.1 主函数
// ============================================================================
int lesson6_1_main()
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Learning - Lesson 6: Camera System", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 告诉 GLFW 捕获鼠标（隐藏鼠标光标）
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ========================================================================
    // 第三步：初始化 GLAD
    // ========================================================================
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ========================================================================
    // 第四步：配置全局 OpenGL 状态
    // ========================================================================
    // 启用深度测试，这样 OpenGL 就能正确处理物体的前后关系
    glEnable(GL_DEPTH_TEST);

    // ========================================================================
    // 第五步：创建和编译着色器程序（使用 Shader 类）
    // ========================================================================
    std::string vertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson6/6.1.camera.vs";
    std::string fragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson6/6.1.camera.fs";
    Shader ourShader(vertexPath.c_str(), fragmentPath.c_str());

    // ========================================================================
    // 第六步：设置顶点数据
    // ========================================================================
    // 顶点数据格式：位置(3) + 纹理坐标(2) = 5 个 float
    // 每个顶点有 5 个 float，总共 20 字节
    // 这是一个立方体的顶点数据（6个面，每个面2个三角形，共36个顶点）
    // ========================================================================
    float vertices[] = {
        // 位置 (x, y, z)        // 纹理坐标 (u, v)
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
    
    // 世界空间中立方体的位置
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 位置属性（location = 0）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 纹理坐标属性（location = 1）
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ========================================================================
    // 第七步：加载纹理
    // ========================================================================
    unsigned int texture1, texture2;
    
    // 纹理 1：尝试加载 wall.jpg，如果失败则使用程序生成的纹理
    std::string texturePath1 = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/wall.jpg";
    texture1 = loadTexture(texturePath1.c_str());
    if (texture1 == 0) {
        std::cout << "警告：无法加载 wall.jpg，使用程序生成的纹理" << std::endl;
        texture1 = createProceduralTexture();
    }
    
    // 纹理 2：创建程序生成的纹理（用于混合效果）
    texture2 = createProceduralTexture();
    
    // 告诉 OpenGL 每个采样器对应哪个纹理单元
    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    // ========================================================================
    // 第八步：渲染循环
    // ========================================================================
    while (!glfwWindowShouldClose(window))
    {
        // ====================================================================
        // 计算时间差（用于平滑移动）
        // ====================================================================
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 处理输入（包含相机控制）
        processInputWithCamera(window);

        // 渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 同时清除颜色缓冲和深度缓冲

        // 绑定纹理到对应的纹理单元
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // 激活着色器程序
        ourShader.use();

        // ====================================================================
        // 创建变换矩阵
        // ====================================================================
        // 投影矩阵：使用相机的 Zoom（视野角度）来创建透视投影
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // 视图矩阵：使用相机类获取视图矩阵
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("view", view);

        // ====================================================================
        // 渲染多个立方体
        // ====================================================================
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // 为每个立方体计算模型矩阵
            glm::mat4 model = glm::mat4(1.0f);
            
            // 先平移立方体到世界空间中的位置
            model = glm::translate(model, cubePositions[i]);
            
            // 然后旋转立方体（随时间旋转）
            float time = (float)glfwGetTime();
            float angle = 20.0f * i + time * 50.0f; // 基础角度 + 随时间旋转（每秒50度）
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            
            // 将模型矩阵传递给着色器
            ourShader.setMat4("model", model);

            // 绘制立方体（36 个顶点）
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // 交换缓冲区和轮询事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ========================================================================
    // 第九步：清理资源
    // ========================================================================
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);

    glfwTerminate();
    return 0;
}

