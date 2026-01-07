// ============================================================================
// Lesson 12.2: 模型加载 + 点光源（Model Loading + Point Light）
// ============================================================================
// 本课程学习内容：
// 1. 在模型加载的基础上添加点光源光照
// 2. 点光源的距离衰减效果
// 3. 模型材质与点光源的交互
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
#include "common/model.h"                 // Model 类

// ============================================================================
// Lesson12_2Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson12_2Application : public CameraApplication
{
public:
    Lesson12_2Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 12.2: Model Loading + Point Light")
        , m_lightPos(1.2f, 1.0f, 2.0f)  // 光源位置
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

        // 告诉 stb_image.h 在加载纹理时翻转 y 轴（在加载模型之前）
        stbi_set_flip_vertically_on_load(true);

        // 创建着色器程序
        std::string vertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson12/2.model_loading_point_light.vs";
        std::string fragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson12/2.model_loading_point_light.fs";
        m_shader = new Shader(vertexPath.c_str(), fragmentPath.c_str());

        // 加载模型
        std::string modelPath = std::string(PROJECT_ROOT) + "/engine/assets/models/backpack/backpack.obj";
        m_model = new Model(modelPath);
        
        std::cout << "模型加载完成！" << std::endl;
    }

    // ========================================================================
    // 每帧更新
    // ========================================================================
    virtual void OnUpdate(float deltaTime) override
    {
        // 调用基类的 OnUpdate（处理相机移动）
        CameraApplication::OnUpdate(deltaTime);
        
        // 可以让光源移动（可选）
        // float time = GetTime();
        // m_lightPos.x = 1.0f + sin(time) * 2.0f;
        // m_lightPos.y = sin(time / 2.0f) * 1.0f;
    }

    // ========================================================================
    // 渲染场景
    // ========================================================================
    virtual void OnRender() override
    {
        // 清除颜色缓冲和深度缓冲
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 激活着色器
        m_shader->use();

        // 设置点光源属性
        m_shader->setVec3("light.position", m_lightPos);
        m_shader->setVec3("viewPos", m_camera.Position);

        // 光源属性
        m_shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        m_shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        m_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
        
        // 衰减系数
        m_shader->setFloat("light.constant", 1.0f);
        m_shader->setFloat("light.linear", 0.09f);
        m_shader->setFloat("light.quadratic", 0.032f);

        // 材质属性（shininess）
        m_shader->setFloat("material.shininess", 32.0f);

        // 视图/投影变换
        glm::mat4 projection = glm::perspective(
            glm::radians(m_camera.Zoom), 
            (float)m_width / (float)m_height, 
            0.1f, 
            100.0f
        );
        glm::mat4 view = m_camera.GetViewMatrix();
        m_shader->setMat4("projection", projection);
        m_shader->setMat4("view", view);

        // 渲染加载的模型
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        m_shader->setMat4("model", model);
        m_model->Draw(*m_shader);
    }

    // ========================================================================
    // 清理资源
    // ========================================================================
    virtual void OnCleanup() override
    {
        delete m_shader;
        delete m_model;
    }

private:
    // ========================================================================
    // 成员变量
    // ========================================================================
    Shader* m_shader;        // 着色器
    Model* m_model;         // 模型
    glm::vec3 m_lightPos;    // 光源位置
};

// ============================================================================
// Lesson 12.2 主函数
// ============================================================================
int lesson12_2_main()
{
    Lesson12_2Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}

