// ============================================================================
// Lesson 13.2: 深度缓冲可视化（Depth Buffer Visualization）
// ============================================================================
// 本课程学习内容：
// 1. 深度缓冲的概念
// 2. 如何可视化深度缓冲
// 3. 深度值的范围和含义
// 4. 使用 gl_FragCoord.z 获取深度值
// 5. 深度缓冲的可视化效果
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include "common/camera_application.h"  // CameraApplication 基类
#include "common/shader.h"               // Shader 类

// ============================================================================
// Lesson13_2Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson13_2Application : public CameraApplication
{
public:
    Lesson13_2Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 13.2: Depth Buffer Visualization")
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

        // 创建着色器程序（用于深度可视化）
        std::string vertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson13/2.depth_visualization.vs";
        std::string fragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson13/2.depth_visualization.fs";
        m_shader = new Shader(vertexPath.c_str(), fragmentPath.c_str());

        // 设置顶点数据（多个立方体）
        SetupVertices();
        
        std::cout << "========================================\n";
        std::cout << "Lesson 13.2: 深度缓冲可视化\n";
        std::cout << "========================================\n";
        std::cout << "白色 = 近处（深度值小）\n";
        std::cout << "黑色 = 远处（深度值大）\n";
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

        // 渲染多个立方体（不同位置，用于观察深度变化）
        glBindVertexArray(m_VAO);
        
        // 立方体位置数组
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

        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            m_shader->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    // ========================================================================
    // 清理资源
    // ========================================================================
    virtual void OnCleanup() override
    {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
        delete m_shader;
    }

private:
    // ========================================================================
    // 设置顶点数据
    // ========================================================================
    void SetupVertices()
    {
        // 立方体的顶点数据（位置）
        float vertices[] = {
            // 前面
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            
            // 后面
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            
            // 左面
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            
            // 右面
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            
            // 底面
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,
            
            // 顶面
            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f
        };

        // 创建 VAO 和 VBO
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // 位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    // ========================================================================
    // 成员变量
    // ========================================================================
    Shader* m_shader;
    unsigned int m_VAO, m_VBO;
};

// ============================================================================
// Lesson 13.2 主函数
// ============================================================================
int lesson13_2_main()
{
    Lesson13_2Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}

