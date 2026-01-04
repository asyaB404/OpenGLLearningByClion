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
// 键盘输入处理（相机移动）
// ============================================================================
void CameraApplication::OnKey(int key, int scancode, int action, int mods)
{
    // 先调用基类处理（ESC 退出等）
    Application::OnKey(key, scancode, action, mods);

    // 处理相机移动（WASD）
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_W)
            m_camera.ProcessKeyboard(FORWARD, GetDeltaTime());
        if (key == GLFW_KEY_S)
            m_camera.ProcessKeyboard(BACKWARD, GetDeltaTime());
        if (key == GLFW_KEY_A)
            m_camera.ProcessKeyboard(LEFT, GetDeltaTime());
        if (key == GLFW_KEY_D)
            m_camera.ProcessKeyboard(RIGHT, GetDeltaTime());
    }
}

// ============================================================================
// 鼠标移动处理（相机旋转）
// ============================================================================
void CameraApplication::OnMouseMove(double xpos, double ypos)
{
    if (!m_mouseCaptured)
        return;

    Application::OnMouseMove(xpos, ypos);

    // 计算鼠标偏移量
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

