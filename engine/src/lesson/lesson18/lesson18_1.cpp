// ============================================================================
// Lesson 18: 几何着色器（Geometry Shader）
// ============================================================================
// 本课程学习内容：
// 1. 几何着色器的概念和作用
// 2. 如何在顶点与片段着色器之间插入几何着色器
// 3. 几何着色器的输入/输出（图元类型、顶点数）
// 4. “沿法线爆炸”等简单几何变形
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
// Lesson18Application
// ============================================================================
class Lesson18Application : public CameraApplication
{
public:
    Lesson18Application()
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 18: Geometry Shader")
    {
    }

protected:
    virtual void OnInitialize() override
    {
        CameraApplication::OnInitialize();
        stbi_set_flip_vertically_on_load(true);

        std::string base = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson18/";
        std::string vs = base + "9.2.geometry_shader.vs";
        std::string fs = base + "9.2.geometry_shader.fs";
        std::string gs = base + "9.2.geometry_shader.gs";
        m_shader = new Shader(vs.c_str(), fs.c_str(), gs.c_str());

        std::string modelPath = std::string(PROJECT_ROOT) + "/engine/assets/models/backpack/backpack.obj";
        m_model = new Model(modelPath);

        std::cout << "========================================\n";
        std::cout << "Lesson 18: 几何着色器\n";
        std::cout << "========================================\n";
        std::cout << "使用 WASD 移动、鼠标旋转、滚轮缩放\n";
        std::cout << "模型三角形会沿法线做周期性的“爆炸”动画\n";
        std::cout << "========================================\n";
    }

    virtual void OnUpdate(float deltaTime) override
    {
        CameraApplication::OnUpdate(deltaTime);
    }

    virtual void OnRender() override
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_shader->use();
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

        m_shader->setMat4("projection", projection);
        m_shader->setMat4("view", view);
        m_shader->setMat4("model", model);
        m_shader->setFloat("time", GetTime());

        m_model->Draw(*m_shader);
    }

    virtual void OnCleanup() override
    {
        delete m_shader;
        delete m_model;
    }

private:
    Shader* m_shader;
    Model* m_model;
};

// ============================================================================
// Lesson 18 主函数
// ============================================================================
int lesson18_1_main()
{
    Lesson18Application app;
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    app.Run();
    app.Cleanup();
    return 0;
}
