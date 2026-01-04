// ============================================================================
// Application 类 - OpenGL 应用程序基类
// ============================================================================
// 这个类封装了 OpenGL 应用程序的基本功能：
// 1. GLFW 窗口的创建和管理
// 2. GLAD 初始化
// 3. 输入处理（键盘、鼠标、滚轮）
// 4. 渲染循环
// 5. 时间管理（deltaTime）
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

// 前向声明
class Camera;

// ============================================================================
// Application 类
// ============================================================================
class Application
{
public:
    // ========================================================================
    // 构造函数和析构函数
    // ========================================================================
    Application(unsigned int width = 800, unsigned int height = 600, 
                const std::string& title = "OpenGL Application");
    virtual ~Application();

    // ========================================================================
    // 主要接口
    // ========================================================================
    // 初始化应用程序（创建窗口、初始化 GLAD 等）
    bool Initialize();
    
    // 运行应用程序（主循环）
    void Run();
    
    // 清理资源
    void Cleanup();

    // ========================================================================
    // 获取窗口信息
    // ========================================================================
    GLFWwindow* GetWindow() const { return m_window; }
    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }
    float GetDeltaTime() const { return m_deltaTime; }
    float GetTime() const { return static_cast<float>(glfwGetTime()); }

protected:
    // ========================================================================
    // 虚函数：子类可以重写这些函数来自定义行为
    // ========================================================================
    
    // 初始化场景（在窗口创建后调用）
    virtual void OnInitialize() {}
    
    // 每帧更新（在渲染前调用）
    virtual void OnUpdate(float deltaTime) {}
    
    // 渲染场景
    virtual void OnRender() {}
    
    // 清理资源（在窗口关闭前调用）
    virtual void OnCleanup() {}
    
    // 窗口大小改变回调
    virtual void OnFramebufferSize(int width, int height);
    
    // 键盘输入处理
    virtual void OnKey(int key, int scancode, int action, int mods);
    
    // 鼠标移动处理
    virtual void OnMouseMove(double xpos, double ypos);
    
    // 鼠标滚轮处理
    virtual void OnMouseScroll(double xoffset, double yoffset);
    
    // 设置鼠标捕获模式
    void SetMouseCaptured(bool captured);

    // ========================================================================
    // 成员变量
    // ========================================================================
    unsigned int m_width;
    unsigned int m_height;
    std::string m_title;
    GLFWwindow* m_window;
    
    // 时间管理
    float m_deltaTime;
    float m_lastFrame;
    
    // 鼠标状态
    bool m_firstMouse;
    float m_lastX;
    float m_lastY;
    bool m_mouseCaptured;

private:
    // ========================================================================
    // 静态回调函数（GLFW 需要 C 风格函数）
    // ========================================================================
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    
    // 从窗口指针获取 Application 实例
    static Application* GetApplication(GLFWwindow* window);
};

