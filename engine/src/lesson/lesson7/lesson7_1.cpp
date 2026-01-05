// ============================================================================
// Lesson 7.1: 基础光照（Basic Lighting）
// ============================================================================
// 本课程学习内容：
// 1. 理解光照的基本概念
// 2. 物体颜色和光源颜色的概念
// 3. 渲染被光照的物体和光源本身
// 4. 使用多个 VAO 渲染不同类型的物体
// 5. 使用不同的着色器渲染不同的物体
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
// Lesson7Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson7Application : public CameraApplication
{
public:
    Lesson7Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 7: Basic Lighting")
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

        // 创建着色器程序
        std::string lightingVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson7/1.colors.vs";
        std::string lightingFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson7/1.colors.fs";
        m_lightingShader = new Shader(lightingVertexPath.c_str(), lightingFragmentPath.c_str());

        std::string lightCubeVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson7/1.light_cube.vs";
        std::string lightCubeFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson7/1.light_cube.fs";
        m_lightCubeShader = new Shader(lightCubeVertexPath.c_str(), lightCubeFragmentPath.c_str());

        // 设置顶点数据
        SetupVertices();
    }

    // ========================================================================
    // 每帧更新
    // ========================================================================
    virtual void OnUpdate(float deltaTime) override
    {
        // 调用基类的 OnUpdate（处理相机移动）
        CameraApplication::OnUpdate(deltaTime);
        
        // 可以在这里更新场景状态
    }

    // ========================================================================
    // 渲染场景
    // ========================================================================
    virtual void OnRender() override
    {
        // 清除颜色缓冲和深度缓冲
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ====================================================================
        // 渲染被光照的立方体
        // ====================================================================
        m_lightingShader->use();
        
        // 设置物体颜色和光源颜色
        m_lightingShader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);  // 橙色
        m_lightingShader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);   // 白色光源

        // 设置投影和视图矩阵
        glm::mat4 projection = glm::perspective(
            glm::radians(m_camera.Zoom), 
            (float)m_width / (float)m_height, 
            0.1f, 
            100.0f
        );
        glm::mat4 view = m_camera.GetViewMatrix();
        m_lightingShader->setMat4("projection", projection);
        m_lightingShader->setMat4("view", view);

        // 设置模型矩阵（立方体在原点）
        glm::mat4 model = glm::mat4(1.0f);
        m_lightingShader->setMat4("model", model);

        // 绘制立方体
        glBindVertexArray(m_cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // ====================================================================
        // 渲染光源立方体
        // ====================================================================
        m_lightCubeShader->use();
        m_lightCubeShader->setMat4("projection", projection);
        m_lightCubeShader->setMat4("view", view);

        // 光源立方体的模型矩阵：平移到光源位置，并缩小
        model = glm::mat4(1.0f);
        model = glm::translate(model, m_lightPos);
        model = glm::scale(model, glm::vec3(0.2f));  // 缩小到 0.2 倍
        m_lightCubeShader->setMat4("model", model);

        // 绘制光源立方体（使用相同的顶点数据，但不同的 VAO）
        glBindVertexArray(m_lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // ========================================================================
    // 清理资源
    // ========================================================================
    virtual void OnCleanup() override
    {
        glDeleteVertexArrays(1, &m_cubeVAO);
        glDeleteVertexArrays(1, &m_lightCubeVAO);
        glDeleteBuffers(1, &m_VBO);
        delete m_lightingShader;
        delete m_lightCubeShader;
    }

private:
    // ========================================================================
    // 设置顶点数据
    // ========================================================================
    void SetupVertices()
    {
        // 立方体的顶点数据（只有位置，没有颜色和纹理坐标）
        float vertices[] = {
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,

            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,

            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
        };

        // 创建 VBO（两个 VAO 共享）
        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // ====================================================================
        // 创建立方体的 VAO（被光照的物体）
        // ====================================================================
        glGenVertexArrays(1, &m_cubeVAO);
        glBindVertexArray(m_cubeVAO);
        
        // 位置属性（location = 0）
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // ====================================================================
        // 创建光源立方体的 VAO（光源本身）
        // ====================================================================
        // 注意：光源立方体使用相同的 VBO，但有自己的 VAO
        // 这样可以有不同的顶点属性配置（虽然这里配置相同）
        glGenVertexArrays(1, &m_lightCubeVAO);
        glBindVertexArray(m_lightCubeVAO);
        
        // 绑定同一个 VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        
        // 位置属性（location = 0）
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    // ========================================================================
    // 成员变量
    // ========================================================================
    Shader* m_lightingShader;      // 被光照物体的着色器
    Shader* m_lightCubeShader;     // 光源立方体的着色器
    
    unsigned int m_cubeVAO;        // 被光照立方体的 VAO
    unsigned int m_lightCubeVAO;    // 光源立方体的 VAO
    unsigned int m_VBO;            // 顶点缓冲区（两个 VAO 共享）
    
    glm::vec3 m_lightPos = glm::vec3(1.2f, 1.0f, 2.0f);  // 光源位置
};

// ============================================================================
// Lesson 7.1 主函数
// ============================================================================
int lesson7_1_main()
{
    Lesson7Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}

