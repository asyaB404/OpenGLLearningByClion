// ============================================================================
// Lesson 15: 混合（Blending）透明纹理
// ============================================================================
// 本课程学习内容：
// 1. 混合（Blending）的概念和作用
// 2. 如何渲染透明纹理
// 3. 透明物体的排序问题
// 4. 从远到近渲染透明物体
// 5. 混合函数（glBlendFunc）
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include "common/camera_application.h"  // CameraApplication 基类
#include "common/shader.h"               // Shader 类

// ============================================================================
// 辅助函数：加载纹理
// ============================================================================
static unsigned int loadTexture(const char* path, bool flipVertically = true)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    
    // 设置是否垂直翻转图片
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
        // 对于透明纹理（RGBA），使用 GL_CLAMP_TO_EDGE 防止边缘半透明
        // 对于不透明纹理，使用 GL_REPEAT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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
// Lesson15Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson15Application : public CameraApplication
{
public:
    Lesson15Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 15: Blending (Transparent Textures)")
    {
    }

protected:
    // ========================================================================
    // 初始化场景
    // ========================================================================
    virtual void OnInitialize() override
    {
        // 调用基类初始化（启用鼠标捕获）
        CameraApplication::OnInitialize();

        // 配置全局 OpenGL 状态
        // 启用深度测试
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        // 启用混合（Blending）
        glEnable(GL_BLEND);
        // 设置混合函数：源颜色 * 源Alpha + 目标颜色 * (1 - 源Alpha)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // 创建着色器程序
        std::string vertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson15/3.2.blending.vs";
        std::string fragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson15/3.2.blending.fs";
        m_shader = new Shader(vertexPath.c_str(), fragmentPath.c_str());

        // 设置顶点数据
        SetupVertices();
        
        // 加载纹理
        LoadTextures();
        
        // 配置着色器
        m_shader->use();
        m_shader->setInt("texture1", 0);
        
        std::cout << "========================================\n";
        std::cout << "Lesson 15: 混合透明纹理\n";
        std::cout << "========================================\n";
        std::cout << "使用 WASD 移动相机\n";
        std::cout << "使用鼠标旋转视角\n";
        std::cout << "观察透明窗户的渲染效果\n";
        std::cout << "========================================\n";
    }

    // ========================================================================
    // 每帧更新
    // ========================================================================
    virtual void OnUpdate(float deltaTime) override
    {
        // 调用基类的 OnUpdate（处理相机移动）
        CameraApplication::OnUpdate(deltaTime);
    }

    // ========================================================================
    // 渲染场景
    // ========================================================================
    virtual void OnRender() override
    {
        // 清除颜色缓冲和深度缓冲
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活着色器
        m_shader->use();
        
        // 设置投影和视图矩阵
        glm::mat4 projection = glm::perspective(
            glm::radians(m_camera.Zoom), 
            (float)m_width / (float)m_height, 
            0.1f, 
            100.0f
        );
        glm::mat4 view = m_camera.GetViewMatrix();
        m_shader->setMat4("projection", projection);
        m_shader->setMat4("view", view);
        
        // 渲染不透明物体（立方体和地面）
        // 先渲染不透明物体，再渲染透明物体
        
        // 渲染立方体
        glBindVertexArray(m_cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_cubeTexture);
        
        // 第一个立方体
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        m_shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // 第二个立方体
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        m_shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // 渲染地面
        glBindVertexArray(m_planeVAO);
        glBindTexture(GL_TEXTURE_2D, m_floorTexture);
        model = glm::mat4(1.0f);
        m_shader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // 渲染透明窗户（从远到近）
        // 关键：透明物体必须从远到近渲染，才能正确混合
        glBindVertexArray(m_transparentVAO);
        glBindTexture(GL_TEXTURE_2D, m_transparentTexture);
        
        // 按距离排序透明窗户（从远到近）
        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < m_windows.size(); i++)
        {
            float distance = glm::length(m_camera.Position - m_windows[i]);
            sorted[distance] = m_windows[i];
        }
        
        // 从远到近渲染（使用反向迭代器）
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            m_shader->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    }

    // ========================================================================
    // 清理资源
    // ========================================================================
    virtual void OnCleanup() override
    {
        glDeleteVertexArrays(1, &m_cubeVAO);
        glDeleteVertexArrays(1, &m_planeVAO);
        glDeleteVertexArrays(1, &m_transparentVAO);
        glDeleteBuffers(1, &m_cubeVBO);
        glDeleteBuffers(1, &m_planeVBO);
        glDeleteBuffers(1, &m_transparentVBO);
        glDeleteTextures(1, &m_cubeTexture);
        glDeleteTextures(1, &m_floorTexture);
        glDeleteTextures(1, &m_transparentTexture);
        delete m_shader;
    }

private:
    // ========================================================================
    // 设置顶点数据
    // ========================================================================
    void SetupVertices()
    {
        // 立方体顶点数据（位置 + 纹理坐标）
        float cubeVertices[] = {
            // positions          // texture Coords
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
        
        // 地面顶点数据
        float planeVertices[] = {
            // positions          // texture Coords 
             5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

             5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
             5.0f, -0.5f, -5.0f,  2.0f, 2.0f
        };
        
        // 透明窗户顶点数据（四边形）
        float transparentVertices[] = {
            // positions         // texture Coords
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
        };
        
        // 创建立方体 VAO 和 VBO
        glGenVertexArrays(1, &m_cubeVAO);
        glGenBuffers(1, &m_cubeVBO);
        glBindVertexArray(m_cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        
        // 创建地面 VAO 和 VBO
        glGenVertexArrays(1, &m_planeVAO);
        glGenBuffers(1, &m_planeVBO);
        glBindVertexArray(m_planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        
        // 创建透明窗户 VAO 和 VBO
        glGenVertexArrays(1, &m_transparentVAO);
        glGenBuffers(1, &m_transparentVBO);
        glBindVertexArray(m_transparentVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_transparentVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
    }
    
    // ========================================================================
    // 加载纹理
    // ========================================================================
    void LoadTextures()
    {
        // 加载立方体纹理（使用程序生成的纹理作为占位符）
        // 注意：原代码使用 marble.jpg，这里使用程序生成的纹理
        m_cubeTexture = CreateProceduralTexture();
        
        // 加载地面纹理（使用程序生成的纹理作为占位符）
        // 注意：原代码使用 metal.png，这里使用程序生成的纹理
        m_floorTexture = CreateProceduralTexture();
        
        // 加载透明窗户纹理
        std::string windowPath = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/window.png";
        m_transparentTexture = loadTexture(windowPath.c_str(), true);
        
        if (m_transparentTexture == 0)
        {
            std::cout << "Warning: Failed to load window texture, using procedural texture instead" << std::endl;
            m_transparentTexture = CreateProceduralTexture();
        }
    }
    
    // ========================================================================
    // 创建程序生成的纹理（占位符）
    // ========================================================================
    static unsigned int CreateProceduralTexture()
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // 创建一个简单的棋盘纹理
        const int width = 256;
        const int height = 256;
        unsigned char data[width * height * 3];
        
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int index = (y * width + x) * 3;
                bool checker = ((x / 32) + (y / 32)) % 2 == 0;
                data[index + 0] = checker ? 200 : 50;  // R
                data[index + 1] = checker ? 200 : 50;  // G
                data[index + 2] = checker ? 200 : 50;  // B
            }
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        return textureID;
    }
    
    // ========================================================================
    // 成员变量
    // ========================================================================
    Shader* m_shader;
    unsigned int m_cubeVAO, m_planeVAO, m_transparentVAO;
    unsigned int m_cubeVBO, m_planeVBO, m_transparentVBO;
    unsigned int m_cubeTexture, m_floorTexture, m_transparentTexture;
    
    // 透明窗户位置
    std::vector<glm::vec3> m_windows = {
        glm::vec3(-1.5f, 0.0f, -0.48f),
        glm::vec3( 1.5f, 0.0f,  0.51f),
        glm::vec3( 0.0f, 0.0f,  0.7f),
        glm::vec3(-0.3f, 0.0f, -2.3f),
        glm::vec3( 0.5f, 0.0f, -0.6f)
    };
};

// ============================================================================
// Lesson 15 主函数
// ============================================================================
int lesson15_1_main()
{
    Lesson15Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}
