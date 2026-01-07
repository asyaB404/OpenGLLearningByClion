// ============================================================================
// Lesson 11.2: 点光源（Point Light）
// ============================================================================
// 本课程学习内容：
// 1. 点光源的概念和特点
// 2. 点光源与方向光的区别
// 3. 距离衰减（Distance Attenuation）：光线强度随距离衰减
// 4. 衰减系数：constant、linear、quadratic
// 5. 渲染多个立方体，观察点光源的距离衰减效果
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>
#include <string>
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
    
    // 设置是否垂直翻转图片
    stbi_set_flip_vertically_on_load(flipVertically);
    
    // 加载图片
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data)
    {
        // 根据通道数确定格式
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
        
        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // 释放图片数据
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        // 加载失败时，删除已创建的纹理对象并返回 0
        glDeleteTextures(1, &textureID);
        return 0;
    }
    
    return textureID;
}

// ============================================================================
// Lesson11_2Application 类 - 继承自 CameraApplication
// ============================================================================
class Lesson11_2Application : public CameraApplication
{
public:
    Lesson11_2Application() 
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 11.2: Point Light")
        , m_lightIntensity(1.0f)  // 默认光源强度为 1.0
        , m_lightColor(1.0f, 1.0f, 1.0f)  // 默认光源颜色为白色
    {
    }
    
    // ========================================================================
    // 设置光源强度（便捷方法）
    // ========================================================================
    void SetLightIntensity(float intensity)
    {
        m_lightIntensity = intensity;
    }
    
    // ========================================================================
    // 设置光源颜色（便捷方法）
    // ========================================================================
    void SetLightColor(const glm::vec3& color)
    {
        m_lightColor = color;
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
        std::string lightingVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson11/5.2.light_casters.vs";
        std::string lightingFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson11/5.2.light_casters.fs";
        m_lightingShader = new Shader(lightingVertexPath.c_str(), lightingFragmentPath.c_str());

        std::string lightCubeVertexPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson11/5.2.light_cube.vs";
        std::string lightCubeFragmentPath = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson11/5.2.light_cube.fs";
        m_lightCubeShader = new Shader(lightCubeVertexPath.c_str(), lightCubeFragmentPath.c_str());

        // 设置顶点数据
        SetupVertices();
        
        // 加载纹理
        LoadTextures();
        
        // 配置着色器（设置纹理单元）
        m_lightingShader->use();
        m_lightingShader->setInt("material.diffuse", 0);   // 纹理单元 0
        m_lightingShader->setInt("material.specular", 1);  // 纹理单元 1
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
        m_lightingShader->use();
        
        // 设置光源位置（点光源使用位置，而不是方向）
        m_lightingShader->setVec3("light.position", m_lightPos);
        m_lightingShader->setVec3("viewPos", m_camera.Position);

        // m_lightColor *= 1.1f;

        // 光源属性
        // 使用光源强度系数和颜色来计算各分量
        glm::vec3 ambient = m_lightColor * m_lightIntensity * 0.2f;
        glm::vec3 diffuse = m_lightColor * m_lightIntensity * 0.5f;
        glm::vec3 specular = m_lightColor * m_lightIntensity * 1.0f;
        
        m_lightingShader->setVec3("light.ambient", ambient);
        m_lightingShader->setVec3("light.diffuse", diffuse);
        m_lightingShader->setVec3("light.specular", specular);
        
        // 衰减系数（Attenuation）
        // 这些值控制光线强度随距离的衰减
        m_lightingShader->setFloat("light.constant", 1.0f);    // 常数项（通常为 1.0）
        m_lightingShader->setFloat("light.linear", 0.09f);      // 线性项
        m_lightingShader->setFloat("light.quadratic", 0.032f); // 二次项

        // 材质属性（使用纹理贴图，只需要设置 shininess）
        m_lightingShader->setFloat("material.shininess", 32.0f);

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

        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_specularMap);

        // 渲染多个立方体
        glBindVertexArray(m_cubeVAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // 为每个立方体计算模型矩阵
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, m_cubePositions[i]);
            
            // 旋转立方体（每个立方体有不同的角度）
            float time = GetTime();
            float angle = 20.0f * i + time * 50.0f;  // 基础角度 + 随时间旋转
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            
            m_lightingShader->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ====================================================================
        // 渲染光源立方体（点光源有位置，所以需要可视化）
        // ====================================================================
        m_lightCubeShader->use();
        m_lightCubeShader->setMat4("projection", projection);
        m_lightCubeShader->setMat4("view", view);

        // 光源立方体的模型矩阵：平移到光源位置，并缩小
        glm::mat4 model = glm::mat4(1.0f);
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
        glDeleteTextures(1, &m_diffuseMap);
        glDeleteTextures(1, &m_specularMap);
        delete m_lightingShader;
        delete m_lightCubeShader;
    }

private:
    // ========================================================================
    // 设置顶点数据（包含位置、法线和纹理坐标）
    // ========================================================================
    void SetupVertices()
    {
        // 立方体的顶点数据：位置(3) + 法线(3) + 纹理坐标(2) = 8 个 float
        float vertices[] = {
            // 位置 (x, y, z)          法线 (nx, ny, nz)       纹理坐标 (u, v)
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
        };

        // 创建 VBO
        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // ====================================================================
        // 创建立方体的 VAO（被光照的物体）
        // ====================================================================
        glGenVertexArrays(1, &m_cubeVAO);
        glBindVertexArray(m_cubeVAO);
        
        // 位置属性（location = 0）
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // 法线属性（location = 1）
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // 纹理坐标属性（location = 2）
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // ====================================================================
        // 创建光源立方体的 VAO（光源本身）
        // ====================================================================
        glGenVertexArrays(1, &m_lightCubeVAO);
        glBindVertexArray(m_lightCubeVAO);
        
        // 绑定同一个 VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        
        // 位置属性（location = 0）
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    // ========================================================================
    // 加载纹理
    // ========================================================================
    void LoadTextures()
    {
        // 加载漫反射贴图
        std::string diffusePath = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/container2.png";
        m_diffuseMap = loadTexture(diffusePath.c_str());
        if (m_diffuseMap == 0)
        {
            std::cout << "警告：无法加载漫反射贴图 container2.png" << std::endl;
        }
        
        // 加载镜面反射贴图
        std::string specularPath = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/container2_specular.png";
        m_specularMap = loadTexture(specularPath.c_str());
        if (m_specularMap == 0)
        {
            std::cout << "警告：无法加载镜面反射贴图 container2_specular.png" << std::endl;
        }
    }

    // ========================================================================
    // 成员变量
    // ========================================================================
    Shader* m_lightingShader;      // 光照着色器
    Shader* m_lightCubeShader;     // 光源立方体的着色器
    
    unsigned int m_cubeVAO;         // 被光照立方体的 VAO
    unsigned int m_lightCubeVAO;    // 光源立方体的 VAO
    unsigned int m_VBO;             // 顶点缓冲区
    
    unsigned int m_diffuseMap;      // 漫反射贴图
    unsigned int m_specularMap;     // 镜面反射贴图
    
    // 光源属性
    float m_lightIntensity;         // 光源强度系数（默认 1.0）
    glm::vec3 m_lightColor;         // 光源颜色（默认白色）
    glm::vec3 m_lightPos = glm::vec3(1.2f, 1.0f, 2.0f);  // 光源位置
    
    // 多个立方体的位置
    glm::vec3 m_cubePositions[10] = {
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
};

// ============================================================================
// Lesson 11.2 主函数
// ============================================================================
int lesson11_2_main()
{
    Lesson11_2Application app;
    
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Cleanup();
    
    return 0;
}

