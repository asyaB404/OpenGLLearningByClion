// ============================================================================
// CameraApplication 类 - 带相机的应用程序基类
// ============================================================================
// 这个类继承自 Application，添加了相机功能：
// 1. 相机管理
// 2. 相机控制（WASD 移动、鼠标旋转、滚轮缩放）
// ============================================================================

#pragma once

#include "application.h"
#include "camera.h"

// ============================================================================
// CameraApplication 类
// ============================================================================
class CameraApplication : public Application
{
public:
    // ========================================================================
    // 构造函数
    // ========================================================================
    CameraApplication(unsigned int width = 800, unsigned int height = 600, 
                     const std::string& title = "OpenGL Camera Application",
                     glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f));

    // ========================================================================
    // 获取相机
    // ========================================================================
    Camera& GetCamera() { return m_camera; }
    const Camera& GetCamera() const { return m_camera; }

protected:
    // ========================================================================
    // 重写输入处理函数
    // ========================================================================
    virtual void OnKey(int key, int scancode, int action, int mods) override;
    virtual void OnMouseMove(double xpos, double ypos) override;
    virtual void OnMouseScroll(double xoffset, double yoffset) override;
    virtual void OnInitialize() override;
    virtual void OnUpdate(float deltaTime) override;

    // ========================================================================
    // 成员变量
    // ========================================================================
    Camera m_camera;
};

