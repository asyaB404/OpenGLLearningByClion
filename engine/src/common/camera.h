// ============================================================================
// Camera 类 - 相机系统
// ============================================================================
// 这个类封装了相机的所有功能，包括：
// 1. 相机位置和方向
// 2. 键盘控制（WASD 移动）
// 3. 鼠标控制（视角旋转）
// 4. 滚轮控制（缩放）
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// ============================================================================
// 相机移动方向枚举
// ============================================================================
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// ============================================================================
// 默认相机参数
// ============================================================================
const float YAW         = -90.0f;  // 偏航角（初始朝向）
const float PITCH       =  0.0f;  // 俯仰角（初始俯仰）
const float SPEED       =  2.5f;  // 移动速度
const float SENSITIVITY =  0.1f;  // 鼠标灵敏度
const float ZOOM        =  45.0f; // 视野角度（FOV）

// ============================================================================
// Camera 类
// ============================================================================
class Camera
{
public:
    // 相机属性
    glm::vec3 Position;      // 相机位置
    glm::vec3 Front;         // 相机前方向量
    glm::vec3 Up;            // 相机上方向量
    glm::vec3 Right;         // 相机右方向量
    glm::vec3 WorldUp;       // 世界上方向量
    
    // 欧拉角
    float Yaw;               // 偏航角（左右旋转）
    float Pitch;             // 俯仰角（上下旋转）
    
    // 相机选项
    float MovementSpeed;     // 移动速度
    float MouseSensitivity;  // 鼠标灵敏度
    float Zoom;              // 视野角度（FOV）

    // ========================================================================
    // 构造函数（使用向量）
    // ========================================================================
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
           float yaw = YAW, 
           float pitch = PITCH) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
          MovementSpeed(SPEED), 
          MouseSensitivity(SENSITIVITY), 
          Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    
    // ========================================================================
    // 构造函数（使用标量值）
    // ========================================================================
    Camera(float posX, float posY, float posZ, 
           float upX, float upY, float upZ, 
           float yaw, float pitch) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
          MovementSpeed(SPEED), 
          MouseSensitivity(SENSITIVITY), 
          Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // ========================================================================
    // 获取视图矩阵
    // ========================================================================
    // 使用欧拉角和 LookAt 矩阵计算视图矩阵
    // ========================================================================
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // ========================================================================
    // 处理键盘输入
    // ========================================================================
    // 参数：
    //   - direction: 移动方向（FORWARD, BACKWARD, LEFT, RIGHT）
    //   - deltaTime: 帧时间差（用于平滑移动）
    // ========================================================================
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // ========================================================================
    // 处理鼠标移动
    // ========================================================================
    // 参数：
    //   - xoffset: X 轴偏移量
    //   - yoffset: Y 轴偏移量
    //   - constrainPitch: 是否限制俯仰角（防止翻转）
    // ========================================================================
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // 确保俯仰角在合理范围内，防止屏幕翻转
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // 使用更新后的欧拉角更新前、右、上向量
        updateCameraVectors();
    }

    // ========================================================================
    // 处理鼠标滚轮
    // ========================================================================
    // 参数：
    //   - yoffset: 滚轮垂直偏移量
    // ========================================================================
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    // ========================================================================
    // 更新相机向量
    // ========================================================================
    // 根据更新后的欧拉角计算前、右、上向量
    // ========================================================================
    void updateCameraVectors()
    {
        // 计算新的前向量
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        
        // 重新计算右向量和上向量
        // 归一化向量，因为当你向上或向下看时，它们的长度会接近 0，导致移动变慢
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

