// ============================================================================
// Lesson 16: 帧缓冲（Framebuffer）和后期处理
// ============================================================================
// 本课程学习内容：
// 1. 帧缓冲（Framebuffer）的概念和作用
// 2. 如何创建和使用帧缓冲
// 3. 渲染到纹理（Render to Texture）
// 4. 后期处理效果（Post-processing）
// 5. 反色、灰度、核效果等后处理
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
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

// ============================================================================
// Lesson16Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson16Application : public CameraApplication
{
public:
    Lesson16Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 16: Framebuffers & Post-processing")
        , m_currentEffect(0)
    {
    }

protected:
    // ========================================================================
    // 初始化场景
    // ========================================================================
    virtual void OnInitialize() override
    {
        CameraApplication::OnInitialize();

        // 配置全局 OpenGL 状态
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // 创建着色器程序
        std::string screenVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson16/5.1.framebuffers_screen.vs";
        std::string screenFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson16/5.1.framebuffers_screen.fs";
        m_screenShader = new Shader(screenVertexPath.c_str(), screenFragmentPath.c_str());
        
        std::string postVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson16/5.1.framebuffers.vs";
        std::string postFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson16/5.1.framebuffers.fs";
        m_postProcessingShader = new Shader(postVertexPath.c_str(), postFragmentPath.c_str());

        // 设置顶点数据
        SetupVertices();
        
        // 加载纹理
        LoadTextures();
        
        // 创建帧缓冲
        SetupFramebuffer();
        
        // 配置着色器
        m_screenShader->use();
        m_screenShader->setInt("texture1", 0);
        
        m_postProcessingShader->use();
        m_postProcessingShader->setInt("screenTexture", 0);
        m_postProcessingShader->setInt("effect", m_currentEffect);
        
        std::cout << "========================================\n";
        std::cout << "Lesson 16: 帧缓冲和后期处理\n";
        std::cout << "========================================\n";
        std::cout << "使用 WASD 移动相机\n";
        std::cout << "使用鼠标旋转视角\n";
        std::cout << "按 SPACE 切换后期处理效果\n";
        std::cout << "当前效果：正常显示\n";
        std::cout << "========================================\n";
    }

    // ========================================================================
    // 每帧更新
    // ========================================================================
    virtual void OnUpdate(float deltaTime) override
    {
        CameraApplication::OnUpdate(deltaTime);
        
        // 处理空格键切换效果
        GLFWwindow* window = GetWindow();
        if (window)
        {
            static bool spaceKeyPressed = false;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceKeyPressed)
            {
                m_currentEffect = (m_currentEffect + 1) % 4;
                m_postProcessingShader->use();
                m_postProcessingShader->setInt("effect", m_currentEffect);
                
                const char* effectNames[] = {"正常显示", "反色", "灰度", "核效果（边缘检测）"};
                std::cout << "效果切换为：" << effectNames[m_currentEffect] << std::endl;
                
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
        // ====================================================================
        // 第一步：渲染到帧缓冲
        // ====================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
        glEnable(GL_DEPTH_TEST);
        
        // 清除帧缓冲
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // 使用场景着色器渲染场景
        m_screenShader->use();
        
        glm::mat4 projection = glm::perspective(
            glm::radians(m_camera.Zoom), 
            (float)m_width / (float)m_height, 
            0.1f, 
            100.0f
        );
        glm::mat4 view = m_camera.GetViewMatrix();
        m_screenShader->setMat4("projection", projection);
        m_screenShader->setMat4("view", view);
        
        // 渲染立方体
        glBindVertexArray(m_cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_cubeTexture);
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        m_screenShader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        m_screenShader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // 渲染地面
        glBindVertexArray(m_planeVAO);
        glBindTexture(GL_TEXTURE_2D, m_floorTexture);
        model = glm::mat4(1.0f);
        m_screenShader->setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // ====================================================================
        // 第二步：渲染到默认帧缓冲（屏幕）
        // ====================================================================
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);  // 禁用深度测试，因为我们只是渲染一个全屏四边形
        
        // 清除默认帧缓冲
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // 使用后期处理着色器渲染全屏四边形
        m_postProcessingShader->use();
        m_postProcessingShader->setInt("effect", m_currentEffect);  // 更新效果
        glBindVertexArray(m_quadVAO);
        glBindTexture(GL_TEXTURE_2D, m_textureColorBuffer);  // 使用帧缓冲的颜色附件
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    // ========================================================================
    // 清理资源
    // ========================================================================
    virtual void OnCleanup() override
    {
        glDeleteVertexArrays(1, &m_cubeVAO);
        glDeleteVertexArrays(1, &m_planeVAO);
        glDeleteVertexArrays(1, &m_quadVAO);
        glDeleteBuffers(1, &m_cubeVBO);
        glDeleteBuffers(1, &m_planeVBO);
        glDeleteBuffers(1, &m_quadVBO);
        glDeleteTextures(1, &m_cubeTexture);
        glDeleteTextures(1, &m_floorTexture);
        glDeleteTextures(1, &m_textureColorBuffer);
        glDeleteRenderbuffers(1, &m_rbo);
        glDeleteFramebuffers(1, &m_framebuffer);
        delete m_screenShader;
        delete m_postProcessingShader;
    }

private:
    // ========================================================================
    // 设置顶点数据
    // ========================================================================
    void SetupVertices()
    {
        // 立方体顶点数据
        float cubeVertices[] = {
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
        
        // 地面顶点数据
        float planeVertices[] = {
             5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

             5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
            -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
             5.0f, -0.5f, -5.0f,  2.0f, 2.0f
        };
        
        // 全屏四边形顶点数据（用于后期处理）
        float quadVertices[] = {
            // 位置        // 纹理坐标
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f
        };
        
        // 创建立方体 VAO 和 VBO
        glGenVertexArrays(1, &m_cubeVAO);
        glGenBuffers(1, &m_cubeVBO);
        glBindVertexArray(m_cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        
        // 创建地面 VAO 和 VBO
        glGenVertexArrays(1, &m_planeVAO);
        glGenBuffers(1, &m_planeVBO);
        glBindVertexArray(m_planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        
        // 创建全屏四边形 VAO 和 VBO
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        
        glBindVertexArray(0);
    }
    
    // ========================================================================
    // 加载纹理
    // ========================================================================
    void LoadTextures()
    {
        // 使用程序生成的纹理作为占位符
        m_cubeTexture = CreateProceduralTexture();
        m_floorTexture = CreateProceduralTexture();
    }
    
    // ========================================================================
    // 创建程序生成的纹理
    // ========================================================================
    static unsigned int CreateProceduralTexture()
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        const int width = 256;
        const int height = 256;
        unsigned char data[width * height * 3];
        
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int index = (y * width + x) * 3;
                bool checker = ((x / 32) + (y / 32)) % 2 == 0;
                data[index + 0] = checker ? 200 : 50;
                data[index + 1] = checker ? 200 : 50;
                data[index + 2] = checker ? 200 : 50;
            }
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        return textureID;
    }
    
    // ========================================================================
    // 设置帧缓冲
    // ========================================================================
    void SetupFramebuffer()
    {
        // 创建帧缓冲对象
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
        
        // 创建颜色附件纹理
        glGenTextures(1, &m_textureColorBuffer);
        glBindTexture(GL_TEXTURE_2D, m_textureColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        // 将颜色附件附加到帧缓冲
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureColorBuffer, 0);
        
        // 创建渲染缓冲对象（用于深度和模板测试）
        glGenRenderbuffers(1, &m_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        // 将渲染缓冲对象附加到帧缓冲
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
        
        // 检查帧缓冲是否完整
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        }
        
        // 解绑帧缓冲
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    
    // ========================================================================
    // 成员变量
    // ========================================================================
    Shader* m_screenShader;
    Shader* m_postProcessingShader;
    unsigned int m_cubeVAO, m_planeVAO, m_quadVAO;
    unsigned int m_cubeVBO, m_planeVBO, m_quadVBO;
    unsigned int m_cubeTexture, m_floorTexture;
    unsigned int m_framebuffer;
    unsigned int m_textureColorBuffer;
    unsigned int m_rbo;
    int m_currentEffect;  // 当前后期处理效果（0=正常, 1=反色, 2=灰度, 3=核效果）
};

// ============================================================================
// Lesson 16 主函数
// ============================================================================
int lesson16_1_main()
{
    Lesson16Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}
