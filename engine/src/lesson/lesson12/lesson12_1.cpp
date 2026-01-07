// ============================================================================
// Lesson 12: 模型加载（Model Loading）
// ============================================================================
// 本课程学习内容：
// 1. 使用 Assimp 库加载 3D 模型
// 2. Mesh 类的实现和使用
// 3. Model 类的实现和使用
// 4. 加载模型的网格、材质和纹理
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
// Lesson12Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson12Application : public CameraApplication
{
public:
    Lesson12Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 12: Model Loading")
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
        std::string vertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson12/1.model_loading.vs";
        std::string fragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson12/1.model_loading.fs";
        m_shader = new Shader(vertexPath.c_str(), fragmentPath.c_str());

        // 加载模型
        // 注意：您需要将模型文件放在 engine/assets/models/ 目录下
        // 例如：engine/assets/models/backpack/backpack.obj
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
    }

    // ========================================================================
    // 渲染场景
    // ========================================================================
    virtual void OnRender() override
    {
        // 清除颜色缓冲和深度缓冲
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 不要忘记在设置 uniform 之前激活着色器
        m_shader->use();

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
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // 将其向下平移，使其位于场景中心
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));     // 它对于我们的场景来说有点大，所以缩小它
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
    Shader* m_shader;    // 着色器
    Model* m_model;      // 模型
};

// ============================================================================
// Lesson 12 主函数
// ============================================================================
int lesson12_1_main()
{
    Lesson12Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}

