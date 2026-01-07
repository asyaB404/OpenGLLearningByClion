cmake_minimum_required(VERSION 3.15)
project(OpenGLLearning)

set(CMAKE_CXX_STANDARD 20)

# 添加 GLFW 子目录（跨平台：Windows/macOS/Linux 都支持）
add_subdirectory(engine/3rdparty/glfw-3.4)

# 添加 GLAD 子目录（OpenGL 函数加载器）
add_subdirectory(engine/3rdparty/glad)

# 添加 GLM 子目录（OpenGL 数学库）
add_subdirectory(engine/3rdparty/glm)

# 配置 Assimp 选项（在添加子目录之前设置）
# 注意：这些选项必须在 add_subdirectory 之前设置
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Build Assimp tools" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build Assimp tests" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "Build Assimp samples" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Install Assimp" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)  # 构建静态库
set(ASSIMP_NO_EXPORT OFF CACHE BOOL "Disable Assimp export" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "Treat warnings as errors" FORCE)  # 避免编译警告导致失败

# 添加 Assimp 子目录（3D 模型加载库）
# 注意：需要先克隆 assimp 仓库到 engine/3rdparty/assimp
# 命令：cd engine/3rdparty && git clone https://github.com/assimp/assimp.git
add_subdirectory(engine/3rdparty/assimp)

# 查找 OpenGL（跨平台：Windows/macOS/Linux 都支持）
find_package(OpenGL REQUIRED)

# 添加可执行文件
add_executable(OpenGLLearning
        engine/src/main.cpp              # 主入口（选择 lesson）
        engine/src/common/common.cpp      # 公共工具函数
        engine/src/common/stb_image_impl.cpp  # stb_image 实现（单头文件库）
        engine/src/lesson/lesson1/lesson1.cpp   # Lesson 1: 创建窗口和基本渲染循环
        engine/src/lesson/lesson2/lesson2.cpp   # Lesson 2: 绘制第一个三角形
        engine/src/lesson/lesson3/lesson3_1.cpp # Lesson 3.1: 顶点颜色（彩色三角形）
        engine/src/lesson/lesson4/lesson4_1.cpp # Lesson 4: 纹理（Texture）
        engine/src/lesson/lesson5/lesson5_1.cpp # Lesson 5: 坐标系统（Coordinate Systems）
        engine/src/lesson/lesson6/lesson6_1.cpp # Lesson 6: 相机系统（Camera System）
        engine/src/lesson/lesson6/lesson6_oop.cpp # Lesson 6 OOP: 面向对象版本
        engine/src/lesson/lesson7/lesson7_1.cpp # Lesson 7: 基础光照（Basic Lighting）
        engine/src/lesson/lesson8/lesson8_1.cpp # Lesson 8: 基础光照（Phong 模型）
        engine/src/lesson/lesson8/lesson8_2.cpp # Lesson 8.2: 基础光照（Phong 模型 + 动态光源）
        engine/src/lesson/lesson9/lesson9_1.cpp # Lesson 9: 材质系统（Materials）
        engine/src/lesson/lesson10/lesson10_1.cpp # Lesson 10: 光照贴图（Lighting Maps）
        engine/src/lesson/lesson11/lesson11_1.cpp # Lesson 11: 方向光（Directional Light）
        engine/src/lesson/lesson11/lesson11_2.cpp # Lesson 11.2: 点光源（Point Light）
        engine/src/lesson/lesson11/lesson11_3.cpp # Lesson 11.3: 聚光灯（Spotlight）
        engine/src/common/application.cpp       # Application 基类实现
        engine/src/common/camera_application.cpp # CameraApplication 实现
        engine/src/lesson/test/test.cpp
)

# 设置包含目录（让编译器能找到 common.h 和 stb_image.h）
target_include_directories(OpenGLLearning PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/src
        ${CMAKE_SOURCE_DIR}/engine/3rdparty  # stb_image.h
)

# 定义资源路径宏（用于在代码中访问资源文件）
# 这样可以在代码中使用 PROJECT_ROOT 宏来构建资源路径
add_definitions(-DPROJECT_ROOT="${CMAKE_SOURCE_DIR}")

# 链接库（跨平台兼容）
target_link_libraries(OpenGLLearning
        glad          # GLAD 库（会自动传递头文件路径）
        glfw          # GLFW 会自动处理平台特定的依赖
        OpenGL::GL    # CMake 的 OpenGL 包在 Windows/macOS/Linux 都可用
        glm::glm      # GLM 数学库（header-only，会自动传递头文件路径）
        assimp        # Assimp 库（3D 模型加载，会自动传递头文件路径）
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

