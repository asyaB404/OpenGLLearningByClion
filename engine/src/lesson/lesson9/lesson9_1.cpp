// ============================================================================
// Lesson 9.1: 材质系统（Materials）
// ============================================================================
// 本课程学习内容：
// 1. 材质系统：将物体颜色和光源颜色分离
// 2. 材质属性：环境光、漫反射、镜面反射系数和高光指数
// 3. 光源属性：环境光、漫反射、镜面反射颜色
// 4. 动态光源颜色：光源颜色随时间变化
// 5. 使用结构体（struct）组织材质和光源数据
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>  // 用于 sin 函数
#include <iostream>
#include <string>
#include "common/camera_application.h"  // CameraApplication 基类
#include "common/shader.h"               // Shader 类

// ============================================================================
// Lesson9Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson9Application : public CameraApplication
{
public:
    Lesson9Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 9: Materials")
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
        std::string materialsVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson9/3.1.materials.vs";
        std::string materialsFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson9/3.1.materials.fs";
        m_materialsShader = new Shader(materialsVertexPath.c_str(), materialsFragmentPath.c_str());

        std::string lightCubeVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson9/3.1.light_cube.vs";
        std::string lightCubeFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson9/3.1.light_cube.fs";
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
        m_materialsShader->use();
        
        // 设置光源位置和相机位置
        m_materialsShader->setVec3("light.position", m_lightPos);
        m_materialsShader->setVec3("viewPos", m_camera.Position);

        // 光源属性（颜色随时间变化）
        float currentTime = GetTime();
        glm::vec3 lightColor;
        lightColor.x = static_cast<float>(sin(currentTime * 2.0));
        lightColor.y = static_cast<float>(sin(currentTime * 0.7));
        lightColor.z = static_cast<float>(sin(currentTime * 1.3));
        
        // 计算环境光和漫反射颜色（降低影响）
        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);  // 降低漫反射影响
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // 环境光影响更低
        
        // 设置光源属性
        m_materialsShader->setVec3("light.ambient", ambientColor);
        m_materialsShader->setVec3("light.diffuse", diffuseColor);
        m_materialsShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);  // 镜面反射保持白色

        // 材质属性
        m_materialsShader->setVec3("material.ambient", 1.0f, 0.5f, 0.31f);   // 环境光反射：橙色
        m_materialsShader->setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);   // 漫反射：橙色
        m_materialsShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);   // 镜面反射：灰色（降低镜面反射效果）
        m_materialsShader->setFloat("material.shininess", 32.0f);              // 高光指数：32

        // 设置投影和视图矩阵
        glm::mat4 projection = glm::perspective(
            glm::radians(m_camera.Zoom), 
            (float)m_width / (float)m_height, 
            0.1f, 
            100.0f
        );
        glm::mat4 view = m_camera.GetViewMatrix();
        m_materialsShader->setMat4("projection", projection);
        m_materialsShader->setMat4("view", view);

        // 设置模型矩阵（立方体在原点）
        glm::mat4 model = glm::mat4(1.0f);
        m_materialsShader->setMat4("model", model);

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

        // 绘制光源立方体
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
        delete m_materialsShader;
        delete m_lightCubeShader;
    }

private:
    // ========================================================================
    // 设置顶点数据（包含位置和法线）
    // ========================================================================
    void SetupVertices()
    {
        // 立方体的顶点数据：位置(3) + 法线(3) = 6 个 float
        float vertices[] = {
            // 位置 (x, y, z)          法线 (nx, ny, nz)
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // 法线属性（location = 1）
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // ====================================================================
        // 创建光源立方体的 VAO（光源本身）
        // ====================================================================
        glGenVertexArrays(1, &m_lightCubeVAO);
        glBindVertexArray(m_lightCubeVAO);
        
        // 绑定同一个 VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        
        // 位置属性（location = 0）
        // 注意：光源立方体不需要法线，所以只使用位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    // ========================================================================
    // 成员变量
    // ========================================================================
    Shader* m_materialsShader;      // 材质着色器
    Shader* m_lightCubeShader;     // 光源立方体的着色器
    
    unsigned int m_cubeVAO;         // 被光照立方体的 VAO
    unsigned int m_lightCubeVAO;    // 光源立方体的 VAO
    unsigned int m_VBO;             // 顶点缓冲区（两个 VAO 共享）
    
    glm::vec3 m_lightPos = glm::vec3(1.2f, 1.0f, 2.0f);  // 光源位置
};

// ============================================================================
// Lesson 9.1 主函数
// ============================================================================
int lesson9_1_main()
{
    Lesson9Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}

