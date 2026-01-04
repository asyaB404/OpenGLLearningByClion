// ============================================================================
// Lesson 6.1 (OOP版本): 相机系统（Camera System）
// ============================================================================
// 本课程学习内容：
// 1. 使用面向对象的方式组织 OpenGL 代码
// 2. Application 基类封装窗口和渲染循环
// 3. CameraApplication 封装相机功能
// 4. 场景管理
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
// Lesson6Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson6Application : public CameraApplication
{
public:
    Lesson6Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 6: Camera System (OOP)")
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

        // 创建和编译着色器程序
        std::string vertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson6/6.1.camera.vs";
        std::string fragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson6/6.1.camera.fs";
        m_shader = new Shader(vertexPath.c_str(), fragmentPath.c_str());

        // 设置顶点数据
        SetupVertices();

        // 加载纹理
        LoadTextures();

        // 配置着色器
        m_shader->use();
        m_shader->setInt("texture1", 0);
        m_shader->setInt("texture2", 1);
    }

    // ========================================================================
    // 每帧更新
    // ========================================================================
    virtual void OnUpdate(float deltaTime) override
    {
        // 可以在这里更新场景状态
    }

    // ========================================================================
    // 渲染场景
    // ========================================================================
    virtual void OnRender() override
    {
        // 清除颜色缓冲和深度缓冲
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texture2);

        // 使用着色器
        m_shader->use();

        // 设置投影矩阵（使用相机的 Zoom）
        glm::mat4 projection = glm::perspective(
            glm::radians(m_camera.Zoom), 
            (float)m_width / (float)m_height, 
            0.1f, 
            100.0f
        );
        m_shader->setMat4("projection", projection);

        // 设置视图矩阵（使用相机）
        glm::mat4 view = m_camera.GetViewMatrix();
        m_shader->setMat4("view", view);

        // 渲染多个立方体
        glBindVertexArray(m_VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // 计算模型矩阵
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, m_cubePositions[i]);
            
            // 旋转立方体（随时间旋转）
            float time = GetTime();
            float angle = 20.0f * i + time * 50.0f;
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
        glDeleteTextures(1, &m_texture1);
        glDeleteTextures(1, &m_texture2);
        delete m_shader;
    }

private:
    // ========================================================================
    // 设置顶点数据
    // ========================================================================
    void SetupVertices()
    {
        float vertices[] = {
            // 位置 (x, y, z)        // 纹理坐标 (u, v)
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

        // 世界空间中立方体的位置
        m_cubePositions[0] = glm::vec3( 0.0f,  0.0f,  0.0f);
        m_cubePositions[1] = glm::vec3( 2.0f,  5.0f, -15.0f);
        m_cubePositions[2] = glm::vec3(-1.5f, -2.2f, -2.5f);
        m_cubePositions[3] = glm::vec3(-3.8f, -2.0f, -12.3f);
        m_cubePositions[4] = glm::vec3( 2.4f, -0.4f, -3.5f);
        m_cubePositions[5] = glm::vec3(-1.7f,  3.0f, -7.5f);
        m_cubePositions[6] = glm::vec3( 1.3f, -2.0f, -2.5f);
        m_cubePositions[7] = glm::vec3( 1.5f,  2.0f, -2.5f);
        m_cubePositions[8] = glm::vec3( 1.5f,  0.2f, -1.5f);
        m_cubePositions[9] = glm::vec3(-1.3f,  1.0f, -1.5f);

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // 位置属性（location = 0）
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        
        // 纹理坐标属性（location = 1）
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    // ========================================================================
    // 加载纹理
    // ========================================================================
    void LoadTextures()
    {
        // 纹理 1：尝试加载 wall.jpg
        std::string texturePath1 = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/wall.jpg";
        m_texture1 = LoadTexture(texturePath1.c_str());
        if (m_texture1 == 0) {
            std::cout << "警告：无法加载 wall.jpg，使用程序生成的纹理" << std::endl;
            m_texture1 = CreateProceduralTexture();
        }
        
        // 纹理 2：创建程序生成的纹理
        m_texture2 = CreateProceduralTexture();
    }

    // ========================================================================
    // 辅助函数：加载纹理
    // ========================================================================
    unsigned int LoadTexture(const char* path, bool flipVertically = true)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(flipVertically);
        unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
        
        if (data)
        {
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
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Failed to load texture: " << path << std::endl;
            glDeleteTextures(1, &textureID);
            return 0;
        }
        
        return textureID;
    }

    // ========================================================================
    // 辅助函数：创建程序生成的纹理
    // ========================================================================
    unsigned int CreateProceduralTexture()
    {
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        unsigned char data[] = {
            255, 0, 0,     // 红色
            0, 255, 0,     // 绿色
            0, 0, 255,     // 蓝色
            255, 255, 0   // 黄色
        };
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        return texture;
    }

    // ========================================================================
    // 成员变量
    // ========================================================================
    Shader* m_shader;
    unsigned int m_VAO, m_VBO;
    unsigned int m_texture1, m_texture2;
    glm::vec3 m_cubePositions[10];
};

// ============================================================================
// Lesson 6.1 主函数（OOP 版本）
// ============================================================================
int lesson6_1_oop_main()
{
    Lesson6Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}

