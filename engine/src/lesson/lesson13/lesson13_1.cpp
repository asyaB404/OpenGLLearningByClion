// ============================================================================
// Lesson 13.1: 深度测试（Depth Testing）
// ============================================================================
// 本课程学习内容：
// 1. 深度测试的概念和作用
// 2. 为什么需要深度测试
// 3. 深度测试的工作原理
// 4. 有/无深度测试的对比
// 5. 深度测试函数（glDepthFunc）
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include "common/application.h"  // Application 基类
#include "common/shader.h"       // Shader 类

// ============================================================================
// Lesson13_1Application 类 - 继承自 Application
// ============================================================================
class Lesson13_1Application : public Application
{
public:
    Lesson13_1Application() 
        : Application(800, 600, "OpenGL Learning - Lesson 13.1: Depth Testing")
        , m_enableDepthTest(true)  // 默认启用深度测试
    {
    }

protected:
    // ========================================================================
    // 初始化场景
    // ========================================================================
    virtual void OnInitialize() override
    {
        // 注意：基类 Application::Initialize() 已经启用了深度测试
        // 但我们可以在这里控制是否启用
        
        // 创建着色器程序
        std::string vertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson13/1.depth_testing.vs";
        std::string fragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson13/1.depth_testing.fs";
        m_shader = new Shader(vertexPath.c_str(), fragmentPath.c_str());

        // 设置顶点数据（两个立方体，一个在前，一个在后）
        SetupVertices();
        
        std::cout << "========================================\n";
        std::cout << "Lesson 13.1: 深度测试演示\n";
        std::cout << "========================================\n";
        std::cout << "按 SPACE 键切换深度测试的启用/禁用\n";
        std::cout << "观察两个立方体的渲染顺序变化\n";
        std::cout << "========================================\n";
    }

    // ========================================================================
    // 每帧更新
    // ========================================================================
    virtual void OnUpdate(float deltaTime) override
    {
        // 处理键盘输入
        GLFWwindow* window = GetWindow();
        if (window)
        {
            // 按空格键切换深度测试
            static bool spaceKeyPressed = false;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceKeyPressed)
            {
                m_enableDepthTest = !m_enableDepthTest;
                if (m_enableDepthTest)
                {
                    glEnable(GL_DEPTH_TEST);
                    std::cout << "深度测试：已启用" << std::endl;
                }
                else
                {
                    glDisable(GL_DEPTH_TEST);
                    std::cout << "深度测试：已禁用" << std::endl;
                }
                spaceKeyPressed = true;
            }
            else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
            {
                spaceKeyPressed = false;
            }
        }
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
            glm::radians(45.0f), 
            (float)m_width / (float)m_height, 
            0.1f, 
            100.0f
        );
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 5.0f),  // 相机位置
            glm::vec3(0.0f, 0.0f, 0.0f),  // 看向原点
            glm::vec3(0.0f, 1.0f, 0.0f)   // 上方向
        );
        m_shader->setMat4("projection", projection);
        m_shader->setMat4("view", view);

        // 渲染第一个立方体（红色，在 z = -1.0）
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, -1.0f));
        m_shader->setMat4("model", model1);
        m_shader->setVec3("objectColor", 1.0f, 0.0f, 0.0f);  // 红色
        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // 渲染第二个立方体（绿色，在 z = -2.0，更大）
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(0.0f, 0.0f, -2.0f));
        model2 = glm::scale(model2, glm::vec3(1.5f, 1.5f, 1.5f));  // 放大 1.5 倍
        m_shader->setMat4("model", model2);
        m_shader->setVec3("objectColor", 0.0f, 1.0f, 0.0f);  // 绿色
        glDrawArrays(GL_TRIANGLES, 0, 36);
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
    bool m_enableDepthTest;  // 是否启用深度测试
};

// ============================================================================
// Lesson 13.1 主函数
// ============================================================================
int lesson13_1_main()
{
    Lesson13_1Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}

