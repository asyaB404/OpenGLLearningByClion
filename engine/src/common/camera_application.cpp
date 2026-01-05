// ============================================================================
// CameraApplication 类实现
// ============================================================================

#include "camera_application.h"

// ============================================================================
// 构造函数
// ============================================================================
CameraApplication::CameraApplication(unsigned int width, unsigned int height, 
                                    const std::string& title, glm::vec3 cameraPos)
    : Application(width, height, title)
    , m_camera(cameraPos)
{
}

// ============================================================================
// 初始化（启用鼠标捕获）
// ============================================================================
void CameraApplication::OnInitialize()
{
    // 启用鼠标捕获（第一人称视角）
    SetMouseCaptured(true);
}

// ============================================================================
// 键盘输入处理（ESC 退出等）
// ============================================================================
void CameraApplication::OnKey(int key, int scancode, int action, int mods)
{
    // 调用基类处理（ESC 退出等）
    Application::OnKey(key, scancode, action, mods);
}

// ============================================================================
// 每帧更新（处理相机移动）
// ============================================================================
void CameraApplication::OnUpdate(float deltaTime)
{
    // 每帧检查按键状态（持续移动）
    GLFWwindow* window = GetWindow();
    if (window)
    {
        // 水平移动（WASD）
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            m_camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            m_camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            m_camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            m_camera.ProcessKeyboard(RIGHT, deltaTime);
        
        // 垂直移动（空格向上，Shift向下）
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            m_camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
            m_camera.ProcessKeyboard(DOWN, deltaTime);
    }
}

// ============================================================================
// 鼠标移动处理（相机旋转）
// ============================================================================
void CameraApplication::OnMouseMove(double xpos, double ypos)
{
    if (!m_mouseCaptured)
        return;

    // 计算鼠标偏移量
    float xposf = static_cast<float>(xpos);
    float yposf = static_cast<float>(ypos);

    if (m_firstMouse)
    {
        m_lastX = xposf;
        m_lastY = yposf;
        m_firstMouse = false;
        return;  // 第一次移动时不处理，只记录位置
    }

    float xoffset = xposf - m_lastX;
    float yoffset = m_lastY - yposf; // 注意：y 坐标是从下往上的，所以需要反转

    m_lastX = xposf;
    m_lastY = yposf;

    // 更新相机方向
    m_camera.ProcessMouseMovement(xoffset, yoffset);
}

// ============================================================================
// 鼠标滚轮处理（相机缩放）
// ============================================================================
void CameraApplication::OnMouseScroll(double xoffset, double yoffset)
{
    m_camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

