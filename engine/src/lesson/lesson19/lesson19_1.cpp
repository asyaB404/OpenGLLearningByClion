// ============================================================================
// Lesson 19: 实例化渲染（Instancing）
// ============================================================================
// 本课程学习内容：
// 1. 实例化（Instancing）的概念与 glDrawElementsInstanced
// 2. 实例数组与 glVertexAttribDivisor
// 3. 用同一网格、不同模型矩阵一次绘制大量物体（如小行星带）
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
#include <cstdlib>
#include <ctime>
#include "common/camera_application.h"
#include "common/shader.h"
#include "common/model.h"

// ============================================================================
// Lesson19Application
// ============================================================================
class Lesson19Application : public CameraApplication
{
public:
    Lesson19Application()
        : CameraApplication(800, 600, "OpenGL Learning - Lesson 19: Instancing",
                            glm::vec3(0.0f, 0.0f, 155.0f))
    {
    }

protected:
    virtual void OnInitialize() override
    {
        CameraApplication::OnInitialize();
        stbi_set_flip_vertically_on_load(true);

        std::string base = std::string(PROJECT_ROOT) + "/engine/src/lesson/lesson19/";
        m_planetShader = new Shader((base + "10.3.planet.vs").c_str(), (base + "10.3.planet.fs").c_str());
        m_asteroidShader = new Shader((base + "10.3.asteroids.vs").c_str(), (base + "10.3.asteroids.fs").c_str());

        std::string modelBase = std::string(PROJECT_ROOT) + "/engine/assets/models/";
        m_planet = new Model(modelBase + "planet/planet.obj");
        m_rock = new Model(modelBase + "rock/rock.obj");

        // 生成大量小行星的模型矩阵（环形 + 随机位移/缩放/旋转）
        m_amount = 10000;
        m_modelMatrices.resize(m_amount);
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        float radius = 150.0f;
        float offset = 25.0f;
        for (unsigned int i = 0; i < m_amount; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            float angle = (float)i / (float)m_amount * 360.0f;
            float displacement = (std::rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float x = std::sin(angle) * radius + displacement;
            displacement = (std::rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float y = displacement * 0.4f;
            displacement = (std::rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float z = std::cos(angle) * radius + displacement;
            model = glm::translate(model, glm::vec3(x, y, z));

            float scale = static_cast<float>(std::rand() % 20) / 100.0f + 0.05f;
            model = glm::scale(model, glm::vec3(scale));

            float rotAngle = static_cast<float>(std::rand() % 360);
            model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.4f, 0.6f, 0.8f));

            m_modelMatrices[i] = model;
        }

        // 实例数组：将模型矩阵上传到 VBO，并绑定到每个 rock mesh 的 VAO（使用 location 7~10，避免与 Mesh 已有 0~6 冲突）
        glGenBuffers(1, &m_instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, m_amount * sizeof(glm::mat4), m_modelMatrices.data(), GL_STATIC_DRAW);

        for (unsigned int i = 0; i < m_rock->meshes.size(); i++)
        {
            unsigned int VAO = m_rock->meshes[i].VAO;
            glBindVertexArray(VAO);
            // mat4 占 4 个 location，依次为 7, 8, 9, 10
            glEnableVertexAttribArray(7);
            glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
            glEnableVertexAttribArray(8);
            glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
            glEnableVertexAttribArray(9);
            glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(10);
            glVertexAttribPointer(10, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

            glVertexAttribDivisor(7, 1);
            glVertexAttribDivisor(8, 1);
            glVertexAttribDivisor(9, 1);
            glVertexAttribDivisor(10, 1);
            glBindVertexArray(0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        std::cout << "========================================\n";
        std::cout << "Lesson 19: 实例化渲染\n";
        std::cout << "========================================\n";
        std::cout << "行星 1 个 + 小行星 " << m_amount << " 个（单次实例化绘制）\n";
        std::cout << "WASD/鼠标/滚轮 控制相机\n";
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
            0.1f,
            1000.0f
        );
        glm::mat4 view = m_camera.GetViewMatrix();

        // 绘制行星（普通单次绘制）
        m_planetShader->use();
        m_planetShader->setMat4("projection", projection);
        m_planetShader->setMat4("view", view);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        m_planetShader->setMat4("model", model);
        m_planet->Draw(*m_planetShader);

        // 绘制小行星（实例化：一次调用绘制 m_amount 个）
        m_asteroidShader->use();
        m_asteroidShader->setMat4("projection", projection);
        m_asteroidShader->setMat4("view", view);
        m_asteroidShader->setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        if (!m_rock->textures_loaded.empty())
            glBindTexture(GL_TEXTURE_2D, m_rock->textures_loaded[0].id);
        for (unsigned int i = 0; i < m_rock->meshes.size(); i++)
        {
            glBindVertexArray(m_rock->meshes[i].VAO);
            glDrawElementsInstanced(
                GL_TRIANGLES,
                static_cast<unsigned int>(m_rock->meshes[i].indices.size()),
                GL_UNSIGNED_INT,
                0,
                m_amount
            );
            glBindVertexArray(0);
        }
    }

    virtual void OnCleanup() override
    {
        glDeleteBuffers(1, &m_instanceVBO);
        delete m_planetShader;
        delete m_asteroidShader;
        delete m_planet;
        delete m_rock;
    }

private:
    Shader* m_planetShader;
    Shader* m_asteroidShader;
    Model* m_planet;
    Model* m_rock;
    std::vector<glm::mat4> m_modelMatrices;
    unsigned int m_amount;
    unsigned int m_instanceVBO;
};

int lesson19_1_main()
{
    Lesson19Application app;
    if (!app.Initialize())
    {
        std::cout << "Failed to initialize application" << std::endl;
        return -1;
    }
    app.Run();
    app.Cleanup();
    return 0;
}
