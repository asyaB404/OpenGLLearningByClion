// ============================================================================
// Lesson 18-2: 法线可视化（Normal Visualization）
// ============================================================================
// 本课程学习内容：
// 1. 使用几何着色器将每个顶点的法线绘制成线段
// 2. 先绘制模型本色，再叠加法线线框，便于调试法线方向
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include "common/camera_application.h"
#include "common/shader.h"
#include "common/model.h"

// ============================================================================
// Lesson18_2Application
// ============================================================================
class Lesson18_2Application : public CameraApplication
{
public:
    Lesson18_2Application()
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 18-2: Normal Visualization")
    {
    }

protected:
    virtual void OnInitialize() override
    {
        CameraApplication::OnInitialize();
        stbi_set_flip_vertically_on_load(true);

        std::string base = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson18/";
        m_defaultShader = new Shader(
            (base + "9.3.default.vs").c_str(),
            (base + "9.3.default.fs").c_str()
        );
        m_normalShader = new Shader(
            (base + "9.3.normal_visualization.vs").c_str(),
            (base + "9.3.normal_visualization.fs").c_str(),
            (base + "9.3.normal_visualization.gs").c_str()
        );

        std::string modelPath = std::string(PROJECT_ROOT) + "/engine/assets/models/backpack/backpack.obj";
        m_model = new Model(modelPath);

        std::cout << "========================================\n";
        std::cout << "Lesson 18-2: 法线可视化\n";
        std::cout << "========================================\n";
        std::cout << "使用 WASD 移动、鼠标旋转、滚轮缩放\n";
        std::cout << "模型先正常渲染，再叠加绿色法线线段\n";
        std::cout << "========================================\n";
    }

    virtual void OnUpdate(float deltaTime) override
    {
        CameraApplication::OnUpdate(deltaTime);
    }

    virtual void OnRender() override
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(m_camera.Zoom),
            (float)m_width / (float)m_height,
            1.0f,
            100.0f
        );
        glm::mat4 view = m_camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

        // 1. 正常绘制模型
        m_defaultShader->use();
        m_defaultShader->setMat4("projection", projection);
        m_defaultShader->setMat4("view", view);
        m_defaultShader->setMat4("model", model);
        m_model->Draw(*m_defaultShader);

        // 2. 用法线可视化着色器绘制法线线段（叠加）
        m_normalShader->use();
        m_normalShader->setMat4("projection", projection);
        m_normalShader->setMat4("view", view);
        m_normalShader->setMat4("model", model);
        m_normalShader->setVec3("normalColor", 0.0f, 1.0f, 0.0f);  // 绿色
        m_model->Draw(*m_normalShader);
    }

    virtual void OnCleanup() override
    {
        delete m_defaultShader;
        delete m_normalShader;
        delete m_model;
    }

private:
    Shader* m_defaultShader;
    Shader* m_normalShader;
    Model* m_model;
};

// ============================================================================
// Lesson 18-2 主函数
// ============================================================================
int lesson18_2_main()
{
    Lesson18_2Application app;
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    app.Run();
    app.Cleanup();
    return 0;
}
