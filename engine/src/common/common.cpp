// ============================================================================
// 公共工具函数实现
// ============================================================================
// 注意：GLAD 必须在 GLFW 之前包含
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "common.h"

// ============================================================================
// 窗口大小改变回调函数实现
// ============================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // glViewport: 设置 OpenGL 渲染区域的大小和位置
    // 参数：(x, y, width, height)
    //   - (0, 0): 从窗口左下角开始
    //   - width, height: 渲染区域的大小（通常等于窗口大小）
    glViewport(0, 0, width, height);
}

// ============================================================================
// 输入处理函数实现
// ============================================================================
void processInput(GLFWwindow* window) {
    // glfwGetKey: 检查指定按键是否被按下
    // GLFW_KEY_ESCAPE: ESC 键
    // GLFW_PRESS: 按键被按下的状态
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        // 设置窗口应该关闭的标志
        // 这会让主循环中的 glfwWindowShouldClose 返回 true
        glfwSetWindowShouldClose(window, true);
    }
}
