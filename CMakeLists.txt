cmake_minimum_required(VERSION 3.15)
project(OpenGLLearning)

set(CMAKE_CXX_STANDARD 20)

# 添加 GLFW 子目录（跨平台：Windows/macOS/Linux 都支持）
add_subdirectory(engine/3rdparty/glfw-3.4)

# 添加 GLAD 子目录（OpenGL 函数加载器）
add_subdirectory(engine/3rdparty/glad)

# 查找 OpenGL（跨平台：Windows/macOS/Linux 都支持）
find_package(OpenGL REQUIRED)

# 添加可执行文件
add_executable(OpenGLLearning
        engine/src/main.cpp              # 主入口（选择 lesson）
        engine/src/common/common.cpp      # 公共工具函数
        engine/src/lesson1/lesson1.cpp   # Lesson 1: 创建窗口和基本渲染循环
        engine/src/lesson2/lesson2.cpp   # Lesson 2: 绘制第一个三角形
        engine/src/lesson3/lesson3_1.cpp # Lesson 3.1: 顶点颜色（彩色三角形）
)

# 设置包含目录（让编译器能找到 common.h）
target_include_directories(OpenGLLearning PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/src
)

# 链接库（跨平台兼容）
target_link_libraries(OpenGLLearning
        glad          # GLAD 库（会自动传递头文件路径）
        glfw          # GLFW 会自动处理平台特定的依赖
        OpenGL::GL    # CMake 的 OpenGL 包在 Windows/macOS/Linux 都可用
)

# macOS 特定框架（仅 macOS 需要）
if(APPLE)
    target_link_libraries(OpenGLLearning
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
    )
endif()

# Windows 特定设置（如果需要）
if(WIN32)
    # Windows 上 OpenGL::GL 已经包含了 opengl32.lib
    # GLFW 会自动链接必要的 Windows 系统库（如 gdi32, user32 等）
endif()

