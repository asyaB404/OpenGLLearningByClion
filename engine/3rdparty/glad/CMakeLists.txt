# GLAD - OpenGL 函数加载器

# 创建 GLAD 静态库
add_library(glad STATIC
    src/glad.c
)

# 设置头文件路径（PUBLIC：链接 glad 的目标会自动获得）
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

