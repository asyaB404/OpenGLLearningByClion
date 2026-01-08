// ============================================================================
// Lesson 14: 模板缓冲（Stencil Buffer）实现轮廓效果
// ============================================================================
// 本课程学习内容：
// 1. 模板缓冲的概念和作用
// 2. 如何使用模板缓冲实现轮廓效果
// 3. 模板测试函数（glStencilFunc）
// 4. 模板操作（glStencilOp）
// 5. 两遍渲染技术：先写入模板，再绘制轮廓
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
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
// Lesson14Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson14Application : public CameraApplication
{
public:
    Lesson14Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 14: Stencil Buffer Outline")
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
        
        // 启用模板测试
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);  // 模板测试函数：不等于1时通过
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  // 模板操作：失败时保持，通过时替换

        // 创建着色器程序
        std::string normalVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson14/2.stencil_testing.vs";
        std::string normalFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson14/2.stencil_testing.fs";
        m_normalShader = new Shader(normalVertexPath.c_str(), normalFragmentPath.c_str());

        std::string outlineVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson14/2.stencil_testing.vs";
        std::string outlineFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson14/2.stencil_single_color.fs";
        m_outlineShader = new Shader(outlineVertexPath.c_str(), outlineFragmentPath.c_str());

        // 设置顶点数据
        SetupVertices();
        
        // 加载纹理（使用程序生成的纹理，因为资源可能不存在）
        LoadTextures();
        
        // 配置着色器
        m_normalShader->use();
        m_normalShader->setInt("texture1", 0);
        
        std::cout << "========================================\n";
        std::cout << "Lesson 14: 模板缓冲轮廓效果\n";
        std::cout << "========================================\n";
        std::cout << "立方体周围有青色轮廓\n";
        std::cout << "使用 WASD 移动相机观察效果\n";
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
        // 清除颜色缓冲、深度缓冲和模板缓冲
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // 设置投影和视图矩阵
        glm::mat4 view = m_camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(m_camera.Zoom), 
            (float)m_width / (float)m_height, 
            0.1f, 
            100.0f
        );

        // ====================================================================
        // 第一遍渲染：绘制地板（不写入模板缓冲）
        // ====================================================================
        glStencilMask(0x00);  // 禁用模板写入（地板不写入模板缓冲）
        
        m_normalShader->use();
        m_normalShader->setMat4("view", view);
        m_normalShader->setMat4("projection", projection);
        
        // 绘制地板
        glBindVertexArray(m_planeVAO);
        glBindTexture(GL_TEXTURE_2D, m_floorTexture);
        m_normalShader->setMat4("model", glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // ====================================================================
        // 第二遍渲染：绘制立方体（写入模板缓冲，值为1）
        // ====================================================================
        glStencilFunc(GL_ALWAYS, 1, 0xFF);  // 总是通过模板测试
        glStencilMask(0xFF);                // 启用模板写入
        
        glBindVertexArray(m_cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_cubeTexture);
        
        // 绘制第一个立方体
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        m_normalShader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // 绘制第二个立方体
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        m_normalShader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ====================================================================
        // 第三遍渲染：绘制轮廓（只在模板值不等于1的地方绘制）
        // ====================================================================
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);  // 模板值不等于1时通过
        glStencilMask(0x00);                  // 禁用模板写入
        glDisable(GL_DEPTH_TEST);             // 禁用深度测试（确保轮廓可见）
        
        m_outlineShader->use();
        m_outlineShader->setMat4("view", view);
        m_outlineShader->setMat4("projection", projection);
        
        float scale = 1.1f;  // 放大 10%
        
        // 绘制第一个立方体的轮廓
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        model = glm::scale(model, glm::vec3(scale, scale, scale));
        m_outlineShader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // 绘制第二个立方体的轮廓
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(scale, scale, scale));
        m_outlineShader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // 恢复状态
        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 0, 0xFF);
        glEnable(GL_DEPTH_TEST);
    }

    // ========================================================================
    // 清理资源
    // ========================================================================
    virtual void OnCleanup() override
    {
        glDeleteVertexArrays(1, &m_cubeVAO);
        glDeleteVertexArrays(1, &m_planeVAO);
        glDeleteBuffers(1, &m_cubeVBO);
        glDeleteBuffers(1, &m_planeVBO);
        glDeleteTextures(1, &m_cubeTexture);
        glDeleteTextures(1, &m_floorTexture);
        delete m_normalShader;
        delete m_outlineShader;
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

        // 地板顶点数据（位置 + 纹理坐标）
        float planeVertices[] = {
            // positions          // texture Coords
             5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

             5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
             5.0f, -0.5f, -5.0f,  2.0f, 2.0f
        };

        // ====================================================================
        // 创建立方体的 VAO 和 VBO
        // ====================================================================
        glGenVertexArrays(1, &m_cubeVAO);
        glGenBuffers(1, &m_cubeVBO);
        glBindVertexArray(m_cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        
        // 位置属性
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        
        // 纹理坐标属性
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);

        // ====================================================================
        // 创建地板的 VAO 和 VBO
        // ====================================================================
        glGenVertexArrays(1, &m_planeVAO);
        glGenBuffers(1, &m_planeVBO);
        glBindVertexArray(m_planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
        
        // 位置属性
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        
        // 纹理坐标属性
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
    }

    // ========================================================================
    // 加载纹理
    // ========================================================================
    void LoadTextures()
    {
        // 尝试加载纹理，如果失败则使用程序生成的纹理
        std::string cubeTexturePath = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/wall.jpg";
        m_cubeTexture = loadTexture(cubeTexturePath.c_str());
        
        if (m_cubeTexture == 0)
        {
            // 如果加载失败，创建程序生成的纹理
            m_cubeTexture = CreateProceduralTexture();
            std::cout << "使用程序生成的纹理（立方体）" << std::endl;
        }
        
        std::string floorTexturePath = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/wall.jpg";
        m_floorTexture = loadTexture(floorTexturePath.c_str());
        
        if (m_floorTexture == 0)
        {
            // 如果加载失败，创建程序生成的纹理
            m_floorTexture = CreateProceduralTexture();
            std::cout << "使用程序生成的纹理（地板）" << std::endl;
        }
    }

    // ========================================================================
    // 创建程序生成的纹理
    // ========================================================================
    unsigned int CreateProceduralTexture()
    {
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        // 创建一个简单的 2x2 纹理（棋盘格图案）
        unsigned char data[] = {
            200, 200, 200,     // 浅灰色
            100, 100, 100,     // 深灰色
            100, 100, 100,     // 深灰色
            200, 200, 200      // 浅灰色
        };
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        return texture;
    }

    // ========================================================================
    // 成员变量
    // ========================================================================
    Shader* m_normalShader;      // 正常渲染着色器
    Shader* m_outlineShader;      // 轮廓渲染着色器
    
    unsigned int m_cubeVAO, m_cubeVBO;    // 立方体 VAO 和 VBO
    unsigned int m_planeVAO, m_planeVBO;  // 地板 VAO 和 VBO
    
    unsigned int m_cubeTexture;          // 立方体纹理
    unsigned int m_floorTexture;         // 地板纹理
};

// ============================================================================
// Lesson 14 主函数
// ============================================================================
int lesson14_1_main()
{
    Lesson14Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}

