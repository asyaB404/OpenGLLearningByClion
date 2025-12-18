// ============================================================================
// Lesson 3.1: 顶点颜色（每个顶点有不同颜色）
// ============================================================================
// 本课程学习内容：
// 1. 如何为每个顶点设置颜色
// 2. 顶点着色器如何传递颜色到片段着色器
// 3. 片段着色器如何接收和使用颜色
// 4. 顶点属性的交错存储（位置和颜色交错）
// 5. 使用 glDrawArrays 绘制（不使用索引）
// ============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "common/common.h"  // 公共工具函数（回调函数和输入处理）

// ============================================================================
// 全局常量定义
// ============================================================================
// static: 使变量只在当前文件内可见，避免与其他文件的同名变量冲突
static const unsigned int SCR_WIDTH = 800;
static const unsigned int SCR_HEIGHT = 600;

// ============================================================================
// 顶点着色器源码（GLSL 语言）
// ============================================================================
// 这个着色器接收两个属性：
// 1. aPos (位置) - location = 0
// 2. aColor (颜色) - location = 1
// 
// 顶点着色器将颜色传递给片段着色器
// ============================================================================
// static: 使变量只在当前文件内可见，避免与其他文件的同名变量冲突
static const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"      // 输入：顶点位置（3D 向量）
    "layout (location = 1) in vec3 aColor;\n"    // 输入：顶点颜色（3D 向量，RGB）
    "out vec3 ourColor;\n"                        // 输出：传递给片段着色器的颜色
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"        // 设置顶点位置（简写形式）
    "   ourColor = aColor;\n"                     // 将颜色传递给片段着色器
    "}\0";

// 详细解释：
// - "layout (location = 1)": 第二个顶点属性的位置索引
// - "in vec3 aColor": 输入变量，类型为 3D 向量（RGB 颜色）
// - "out vec3 ourColor": 输出变量，传递给片段着色器
// - 顶点着色器的输出会自动插值后传递给片段着色器
//
// 关键：顶点着色器为每个顶点设置颜色，然后 OpenGL 会在三角形内部
//       自动插值这些颜色，形成平滑的渐变效果

// ============================================================================
// 片段着色器源码（GLSL 语言）
// ============================================================================
// 这个着色器接收从顶点着色器传来的颜色
// 并对每个像素（片段）应用插值后的颜色
// ============================================================================
// static: 使变量只在当前文件内可见，避免与其他文件的同名变量冲突
static const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"                       // 输出：最终片段颜色（4D 向量，RGBA）
    "in vec3 ourColor;\n"                         // 输入：从顶点着色器传来的颜色（已插值）
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0f);\n"     // 设置片段颜色（添加 Alpha = 1.0）
    "}\n\0";

// 详细解释：
// - "in vec3 ourColor": 输入变量，从顶点着色器接收颜色
// - 颜色会在三角形内部自动插值（渐变）
// - 例如：三个顶点分别是红、绿、蓝，三角形内部会有渐变色
//
// 插值过程（OpenGL 自动完成）：
// 1. 顶点着色器输出三个顶点的颜色：红、绿、蓝
// 2. 光栅化时，OpenGL 根据像素位置计算：
//    - 距离红色顶点近 → 颜色偏红
//    - 距离绿色顶点近 → 颜色偏绿
//    - 距离蓝色顶点近 → 颜色偏蓝
//    - 在中间位置 → 三种颜色的混合
// 3. 片段着色器接收插值后的颜色并输出
// 4. 最终效果：平滑的彩色渐变三角形

// ============================================================================
// Lesson 3.1 主函数
// ============================================================================
int lesson3_1_main()
{
    // ========================================================================
    // 第一步：初始化 GLFW
    // ========================================================================
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // ========================================================================
    // 第二步：创建窗口
    // ========================================================================
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Learning - Lesson 3.1: 顶点颜色", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ========================================================================
    // 第三步：初始化 GLAD
    // ========================================================================
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ========================================================================
    // 第四步：创建和编译着色器程序
    // ========================================================================
    // 顶点着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    // 检查编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // 片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // 检查编译错误
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // 链接着色器程序
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // 检查链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ========================================================================
    // 第五步：设置顶点数据
    // ========================================================================
    // 顶点数据格式：位置(3个float) + 颜色(3个float)
    // 这种存储方式叫做"交错存储"（Interleaved）
    // 
    // 每个顶点的数据：
    // - 位置：x, y, z（3个float）
    // - 颜色：R, G, B（3个float）
    // 总共：6个float = 24字节
    // ========================================================================
    // ========================================================================
    // 顶点数据：每个顶点包含位置和颜色
    // ========================================================================
    // 为什么是彩色的？关键在这里：
    // 1. 每个顶点都有自己的颜色
    // 2. 顶点着色器接收颜色并传递给片段着色器
    // 3. OpenGL 在三角形内部自动插值颜色（渐变）
    // 4. 片段着色器使用插值后的颜色渲染每个像素
    // ========================================================================
    float vertices[] = {
        // 位置 (x, y, z)        // 颜色 (R, G, B)
         0.5f, -0.5f, 0.0f,     1.0f, 0.0f, 0.0f,  // 右下角顶点：红色 (R=1.0, G=0.0, B=0.0)
        -0.5f, -0.5f, 0.0f,     0.0f, 1.0f, 0.0f,  // 左下角顶点：绿色 (R=0.0, G=1.0, B=0.0)
         0.0f,  0.5f, 0.0f,     0.0f, 0.0f, 1.0f   // 顶部顶点：蓝色 (R=0.0, G=0.0, B=1.0)
    };
    
    // ========================================================================
    // 颜色插值原理（为什么三角形内部是渐变色）
    // ========================================================================
    // OpenGL 的渲染流程：
    // 
    // 1. 顶点着色器阶段：
    //    - 处理 3 个顶点，每个顶点有颜色
    //    - 顶点 0: 红色 (1.0, 0.0, 0.0)
    //    - 顶点 1: 绿色 (0.0, 1.0, 0.0)
    //    - 顶点 2: 蓝色 (0.0, 0.0, 1.0)
    //    - 顶点着色器将颜色传递给片段着色器
    //
    // 2. 光栅化阶段（自动插值）：
    //    - OpenGL 将三角形转换为像素（片段）
    //    - 对于三角形内部的每个像素，OpenGL 会根据：
    //      * 像素到三个顶点的距离
    //      * 三个顶点的颜色值
    //      自动计算该像素的颜色（插值）
    //
    // 3. 插值示例：
    //    - 靠近红色顶点的像素 → 偏红色
    //    - 靠近绿色顶点的像素 → 偏绿色
    //    - 靠近蓝色顶点的像素 → 偏蓝色
    //    - 三角形中心 → 三种颜色的混合（可能是灰色或白色）
    //
    // 4. 片段着色器阶段：
    //    - 接收插值后的颜色
    //    - 直接输出这个颜色
    //
    // 结果：三角形内部会有从红到绿到蓝的平滑渐变！
    // ========================================================================
    
    // 注意：这个三角形只有 3 个顶点，不需要索引数组
    // 使用 glDrawArrays 直接绘制，而不是 glDrawElements

    // 创建 VAO 和 VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    // 绑定 VAO（之后的所有操作都会记录在这个 VAO 中）
    glBindVertexArray(VAO);

    // 绑定并设置 VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // ========================================================================
    // 设置顶点属性（位置）
    // ========================================================================
    // glVertexAttribPointer: 告诉 OpenGL 如何解释顶点数据
    // 参数：
    //   - 0: 属性位置索引（对应着色器中的 layout(location = 0)）
    //   - 3: 每个属性的分量数量（x, y, z 共 3 个）
    //   - GL_FLOAT: 数据类型（浮点数）
    //   - GL_FALSE: 是否标准化（这里不标准化）
    //   - 6 * sizeof(float): 步长（相邻两个顶点之间的字节数）
    //     * 每个顶点有 6 个 float（3个位置 + 3个颜色）= 24 字节
    //   - (void*)0: 偏移量（位置数据从第 0 个字节开始）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ========================================================================
    // 设置顶点属性（颜色）
    // ========================================================================
    // 参数：
    //   - 1: 属性位置索引（对应着色器中的 layout(location = 1)）
    //   - 3: 每个属性的分量数量（R, G, B 共 3 个）
    //   - GL_FLOAT: 数据类型（浮点数）
    //   - GL_FALSE: 是否标准化（这里不标准化）
    //   - 6 * sizeof(float): 步长（与位置属性相同）
    //   - (void*)(3 * sizeof(float)): 偏移量（颜色数据从第 12 个字节开始）
    //     * 位置数据占 3 个 float = 12 字节，所以颜色从第 12 字节开始
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 激活着色器程序（可以在循环外设置，因为只有一个着色器程序）
    glUseProgram(shaderProgram);

    // ========================================================================
    // 第六步：渲染循环
    // ========================================================================
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入
        processInput(window);

        // 渲染
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 绘制三角形
        // glDrawArrays: 不使用索引，直接按顺序绘制顶点
        // 参数：
        //   - GL_TRIANGLES: 绘制模式（绘制三角形）
        //   - 0: 起始索引（从第 0 个顶点开始）
        //   - 3: 要绘制的顶点数量（3 个顶点组成一个三角形）
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 交换缓冲区和轮询事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ========================================================================
    // 第七步：清理资源
    // ========================================================================
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

