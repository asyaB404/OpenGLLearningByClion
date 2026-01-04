// ============================================================================
// Application 类实现
// ============================================================================

#include "application.h"
#include <iostream>

// ============================================================================
// 构造函数
// ============================================================================
Application::Application(unsigned int width, unsigned int height, const std::string& title)
    : m_width(width)
    , m_height(height)
    , m_title(title)
    , m_window(nullptr)
    , m_deltaTime(0.0f)
    , m_lastFrame(0.0f)
    , m_firstMouse(true)
    , m_lastX(width / 2.0f)
    , m_lastY(height / 2.0f)
    , m_mouseCaptured(false)
{
}

// ============================================================================
// 析构函数
// ============================================================================
Application::~Application()
{
    Cleanup();
}

// ============================================================================
// 初始化应用程序
// ============================================================================
bool Application::Initialize()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // 配置 GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 创建窗口
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
    if (m_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    // 设置用户指针，以便在回调函数中访问 Application 实例
    glfwSetWindowUserPointer(m_window, this);

    // 设置回调函数
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetCursorPosCallback(m_window, MouseCallback);
    glfwSetScrollCallback(m_window, ScrollCallback);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // 启用深度测试
    glEnable(GL_DEPTH_TEST);

    // 调用子类的初始化函数
    OnInitialize();

    return true;
}

// ============================================================================
// 运行应用程序
// ============================================================================
void Application::Run()
{
    if (!m_window)
    {
        std::cout << "Window not initialized. Call Initialize() first." << std::endl;
        return;
    }

    while (!glfwWindowShouldClose(m_window))
    {
        // 计算时间差
        float currentFrame = static_cast<float>(glfwGetTime());
        m_deltaTime = currentFrame - m_lastFrame;
        m_lastFrame = currentFrame;

        // 处理输入
        glfwPollEvents();

        // 更新
        OnUpdate(m_deltaTime);

        // 渲染
        OnRender();

        // 交换缓冲区
        glfwSwapBuffers(m_window);
    }
}

// ============================================================================
// 清理资源
// ============================================================================
void Application::Cleanup()
{
    if (m_window)
    {
        OnCleanup();
        glfwTerminate();
        m_window = nullptr;
    }
}

// ============================================================================
// 窗口大小改变回调
// ============================================================================
void Application::OnFramebufferSize(int width, int height)
{
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}

// ============================================================================
// 键盘输入处理
// ============================================================================
void Application::OnKey(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, true);
    }
}

// ============================================================================
// 鼠标移动处理
// ============================================================================
void Application::OnMouseMove(double xpos, double ypos)
{
    float xposf = static_cast<float>(xpos);
    float yposf = static_cast<float>(ypos);

    if (m_firstMouse)
    {
        m_lastX = xposf;
        m_lastY = yposf;
        m_firstMouse = false;
    }

    float xoffset = xposf - m_lastX;
    float yoffset = m_lastY - yposf; // 注意：y 坐标是从下往上的，所以需要反转

    m_lastX = xposf;
    m_lastY = yposf;
}

// ============================================================================
// 鼠标滚轮处理
// ============================================================================
void Application::OnMouseScroll(double xoffset, double yoffset)
{
    // 默认实现为空，子类可以重写
}

// ============================================================================
// 设置鼠标捕获模式
// ============================================================================
void Application::SetMouseCaptured(bool captured)
{
    m_mouseCaptured = captured;
    if (m_window)
    {
        glfwSetInputMode(m_window, GLFW_CURSOR, 
                        captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        if (captured)
        {
            m_firstMouse = true;
        }
    }
}

// ============================================================================
// 静态回调函数实现
// ============================================================================
void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    Application* app = GetApplication(window);
    if (app)
    {
        app->OnFramebufferSize(width, height);
    }
}

void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Application* app = GetApplication(window);
    if (app)
    {
        app->OnKey(key, scancode, action, mods);
    }
}

void Application::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    Application* app = GetApplication(window);
    if (app)
    {
        app->OnMouseMove(xpos, ypos);
    }
}

void Application::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    Application* app = GetApplication(window);
    if (app)
    {
        app->OnMouseScroll(xoffset, yoffset);
    }
}

// ============================================================================
// 从窗口指针获取 Application 实例
// ============================================================================
Application* Application::GetApplication(GLFWwindow* window)
{
    return static_cast<Application*>(glfwGetWindowUserPointer(window));
}

