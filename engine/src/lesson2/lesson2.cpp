// ============================================================================
// Lesson 2: 绘制第一个三角形（使用着色器和缓冲区）
// ============================================================================
// 本课程学习内容：
// 1. 什么是着色器（Shader）以及如何使用
// 2. 顶点缓冲区（VBO）的作用和使用
// 3. 索引缓冲区（EBO）的作用和使用
// 4. 顶点数组对象（VAO）的作用和使用
// 5. 如何绘制第一个三角形
// ============================================================================

#include <glad/glad.h>     // GLAD：OpenGL 函数加载器
#include <GLFW/glfw3.h>    // GLFW：窗口和输入管理库
#include <iostream>         // 标准输入输出库
#include "../common/common.h"  // 公共工具函数（回调函数和输入处理）

// ============================================================================
// 全局常量定义
// ============================================================================
// 定义窗口的宽度和高度（单位：像素）
// static: 使变量只在当前文件内可见，避免与其他文件的同名变量冲突
static const unsigned int SCR_WIDTH = 800;   // 窗口宽度：800 像素
static const unsigned int SCR_HEIGHT = 600;  // 窗口高度：600 像素

// ============================================================================
// 顶点着色器源码（使用 GLSL 语言编写）
// ============================================================================
// 着色器（Shader）是什么？
// - 着色器是在 GPU 上运行的小程序
// - 顶点着色器：处理每个顶点的位置
// - 片段着色器：处理每个像素的颜色
//
// GLSL（OpenGL Shading Language）是 OpenGL 的着色器编程语言
// ============================================================================
// static: 使变量只在当前文件内可见，避免与其他文件的同名变量冲突
static const char *vertexShaderSource = "#version 330 core\n"  // 指定 GLSL 版本为 3.3
    "layout (location = 0) in vec3 aPos;\n"             // 输入：顶点位置（3D 向量）
    "void main()\n"                                       // 主函数
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"  // 设置顶点位置（转换为 4D）
    "}\0";

// 详细解释：
// - "#version 330 core": 使用 OpenGL 3.3 核心配置文件
// - "layout (location = 0)": 指定输入变量的位置索引为 0
// - "in vec3 aPos": 输入变量，类型为 3D 向量（vec3），名为 aPos（表示位置）
// - "gl_Position": OpenGL 内置变量，用于输出顶点的最终位置（必须是 4D 向量）
// - "vec4(...)": 将 3D 位置转换为 4D（x, y, z, w），w 通常为 1.0

// ============================================================================
// 片段着色器源码（使用 GLSL 语言编写）
// ============================================================================
// 片段着色器的作用：
// - 决定每个像素（片段）的最终颜色
// - 在顶点着色器处理完顶点后运行
// ============================================================================
// static: 使变量只在当前文件内可见，避免与其他文件的同名变量冲突
static const char *fragmentShaderSource = "#version 330 core\n"  // 指定 GLSL 版本为 3.3
    "out vec4 FragColor;\n"                                // 输出：片段颜色（4D 向量，RGBA）
    "void main()\n"                                        // 主函数
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"     // 设置颜色为橙色（R=1.0, G=0.5, B=0.2, A=1.0）
    "}\n\0";

// 详细解释：
// - "out vec4 FragColor": 输出变量，类型为 4D 向量（RGBA 颜色）
// - "vec4(1.0f, 0.5f, 0.2f, 1.0f)": 
//   - R（红色）= 1.0（最大值，最红）
//   - G（绿色）= 0.5（中等值）
//   - B（蓝色）= 0.2（较小值）
//   - A（透明度）= 1.0（完全不透明）
//   - 组合起来就是橙色

// ============================================================================
// Lesson 2 主函数
// ============================================================================
int lesson2_main()
{
    // ========================================================================
    // 第一步：初始化 GLFW（窗口管理库）
    // ========================================================================
    // glfwInit: 初始化 GLFW 库，必须在其他 GLFW 函数之前调用
    glfwInit();
    
    // glfwWindowHint: 设置窗口创建时的属性
    // 设置 OpenGL 主版本号为 3（OpenGL 3.x）
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    
    // 设置 OpenGL 次版本号为 3（OpenGL 3.3）
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    // 设置使用核心配置文件（Core Profile）
    // Core Profile: 只包含现代 OpenGL 功能，不包含已弃用的旧功能
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // macOS 特殊设置：必须启用前向兼容
    // 原因：macOS 只支持 Core Profile + Forward Compatible 模式
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // ========================================================================
    // 第二步：创建窗口
    // ========================================================================
    // glfwCreateWindow: 创建窗口和 OpenGL 上下文
    // 参数：
    //   - SCR_WIDTH, SCR_HEIGHT: 窗口的宽度和高度
    //   - "OpenGL Learning - Lesson 2": 窗口标题
    //   - NULL, NULL: 全屏模式和资源共享（这里不使用）
    // 返回值：窗口指针，如果创建失败返回 NULL
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Learning - Lesson 2", NULL, NULL);
    
    // 检查窗口是否创建成功
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();  // 清理 GLFW 资源
        return -1;         // 返回错误码
    }
    
    // 将当前窗口的 OpenGL 上下文设置为当前线程的主上下文
    // 这样后续的 OpenGL 调用就会作用在这个窗口上
    glfwMakeContextCurrent(window);
    
    // 注册窗口大小改变的回调函数
    // 当窗口大小改变时，会自动调用 framebuffer_size_callback 函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // ========================================================================
    // 第三步：初始化 GLAD（OpenGL 函数加载器）
    // ========================================================================
    // GLAD 的作用：加载 OpenGL 函数的实际地址
    // 因为 OpenGL 是跨平台的，函数地址在运行时才能确定
    // glfwGetProcAddress: GLFW 提供的函数，用于获取 OpenGL 函数的地址
    // gladLoadGLLoader: GLAD 提供的函数，用于加载所有 OpenGL 函数
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;  // 初始化失败，退出程序
    }

    // ========================================================================
    // 第四步：创建和编译着色器程序
    // ========================================================================
    // 着色器程序的工作流程：
    // 1. 创建着色器对象
    // 2. 将源码附加到着色器对象
    // 3. 编译着色器
    // 4. 检查编译是否成功
    // 5. 创建着色器程序
    // 6. 将着色器附加到程序
    // 7. 链接程序
    // 8. 删除着色器对象（已经链接到程序，不再需要）
    // ========================================================================
    
    // ----------------------------------------------------------------
    // 4.1 创建和编译顶点着色器
    // ----------------------------------------------------------------
    // glCreateShader: 创建一个着色器对象
    // GL_VERTEX_SHADER: 指定创建的是顶点着色器
    // 返回值：着色器对象的 ID（如果失败返回 0）
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    // glShaderSource: 将着色器源码附加到着色器对象
    // 参数：
    //   - vertexShader: 着色器对象的 ID
    //   - 1: 源码字符串的数量（这里只有 1 个字符串）
    //   - &vertexShaderSource: 指向源码字符串的指针
    //   - NULL: 字符串长度数组（NULL 表示字符串以 \0 结尾）
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    
    // glCompileShader: 编译着色器
    // 将 GLSL 源码编译成 GPU 可以执行的代码
    glCompileShader(vertexShader);
    
    // 检查编译是否成功
    int success;              // 编译状态（1=成功，0=失败）
    char infoLog[512];        // 错误信息缓冲区
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // 获取编译错误信息
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // ----------------------------------------------------------------
    // 4.2 创建和编译片段着色器
    // ----------------------------------------------------------------
    // 过程与顶点着色器相同，只是类型不同
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
    
    // ----------------------------------------------------------------
    // 4.3 创建着色器程序并链接
    // ----------------------------------------------------------------
    // glCreateProgram: 创建一个着色器程序对象
    // 着色器程序是多个着色器的组合，用于完整的渲染管线
    unsigned int shaderProgram = glCreateProgram();
    
    // glAttachShader: 将着色器附加到程序
    // 一个程序通常包含：顶点着色器 + 片段着色器
    glAttachShader(shaderProgram, vertexShader);   // 附加顶点着色器
    glAttachShader(shaderProgram, fragmentShader); // 附加片段着色器
    
    // glLinkProgram: 链接着色器程序
    // 链接过程会检查：
    // - 顶点着色器的输出是否与片段着色器的输入匹配
    // - 所有变量是否正确连接
    glLinkProgram(shaderProgram);
    
    // 检查链接是否成功
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    // 删除着色器对象（已经链接到程序，不再需要单独保存）
    // 这样可以释放内存
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ========================================================================
    // 第五步：设置顶点数据
    // ========================================================================
    // 在 OpenGL 中，我们需要告诉 GPU 要绘制什么
    // 这里我们定义了一个矩形（由 4 个顶点组成）
    // 然后通过索引数组将其分成两个三角形
    // ========================================================================
    
    // ----------------------------------------------------------------
    // 5.1 定义顶点数据
    // ----------------------------------------------------------------
    // 顶点坐标（在标准化设备坐标中，范围是 -1.0 到 1.0）
    // 格式：(x, y, z) - 每个顶点有 3 个浮点数
    // 
    // 坐标系统：
    //   - x: 右为正，左为负
    //   - y: 上为正，下为负
    //   - z: 前为正，后为负（这里都是 0，表示在屏幕平面上）
    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // 右上角（索引 0）
         0.5f, -0.5f, 0.0f,  // 右下角（索引 1）
        -0.5f, -0.5f, 0.0f,  // 左下角（索引 2）
        -0.5f,  0.5f, 0.0f   // 左上角（索引 3）
    };
    
    // 索引数组（用于定义如何连接顶点形成三角形）
    // 为什么使用索引？
    // - 可以重用顶点，节省内存
    // - 这个矩形有 4 个顶点，但需要 6 个顶点来画两个三角形
    // - 使用索引可以只定义 4 个顶点，然后通过索引重用它们
    unsigned int indices[] = {
        0, 1, 3,  // 第一个三角形：右上角 -> 右下角 -> 左上角
        1, 2, 3   // 第二个三角形：右下角 -> 左下角 -> 左上角
    };
    // 注意：两个三角形共享顶点 1（右下角）和顶点 3（左上角）
    
    // ----------------------------------------------------------------
    // 5.2 创建缓冲区对象
    // ----------------------------------------------------------------
    // VAO（Vertex Array Object，顶点数组对象）
    // - 作用：存储顶点属性配置和 VBO/EBO 的引用
    // - 优点：可以快速切换不同的顶点配置
    //
    // VBO（Vertex Buffer Object，顶点缓冲区对象）
    // - 作用：在 GPU 内存中存储顶点数据
    // - 优点：数据在 GPU 上，访问速度快
    //
    // EBO（Element Buffer Object，索引缓冲区对象）
    // - 作用：在 GPU 内存中存储索引数据
    // - 优点：可以重用顶点，节省内存
    unsigned int VBO, VAO, EBO;  // 存储缓冲区对象的 ID
    
    // glGenVertexArrays: 生成 VAO
    // 参数：
    //   - 1: 要生成的 VAO 数量
    //   - &VAO: 存储生成的 ID 的数组
    glGenVertexArrays(1, &VAO);
    
    // glGenBuffers: 生成缓冲区对象
    // 参数：
    //   - 1: 要生成的缓冲区数量
    //   - &VBO: 存储生成的 ID 的数组
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    // ----------------------------------------------------------------
    // 5.3 绑定并配置缓冲区
    // ----------------------------------------------------------------
    // 绑定 VAO（之后的所有操作都会记录在这个 VAO 中）
    glBindVertexArray(VAO);

    // 绑定 VBO 到 GL_ARRAY_BUFFER 目标
    // GL_ARRAY_BUFFER: 用于存储顶点属性数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // glBufferData: 将数据复制到当前绑定的缓冲区
    // 参数：
    //   - GL_ARRAY_BUFFER: 目标缓冲区类型
    //   - sizeof(vertices): 数据大小（字节数）
    //   - vertices: 指向数据的指针
    //   - GL_STATIC_DRAW: 使用提示
    //     * STATIC: 数据不会或很少改变
    //     * DRAW: 数据由应用程序提供，用于绘制
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 绑定 EBO 到 GL_ELEMENT_ARRAY_BUFFER 目标
    // GL_ELEMENT_ARRAY_BUFFER: 用于存储索引数据
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    
    // 将索引数据复制到 EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // ----------------------------------------------------------------
    // 5.4 设置顶点属性
    // ----------------------------------------------------------------
    // glVertexAttribPointer: 告诉 OpenGL 如何解释顶点数据
    // 参数：
    //   - 0: 顶点属性的位置索引（对应着色器中的 layout(location = 0)）
    //   - 3: 每个顶点属性的分量数量（x, y, z 共 3 个）
    //   - GL_FLOAT: 数据类型（浮点数）
    //   - GL_FALSE: 是否标准化（这里不标准化）
    //   - 3 * sizeof(float): 步长（相邻两个顶点之间的字节数）
    //     * 3 个 float = 12 字节
    //   - (void*)0: 偏移量（从数据开始到第一个属性的字节数，这里是 0）
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    // glEnableVertexAttribArray: 启用顶点属性
    // 参数：0 - 要启用的属性位置索引
    // 只有启用的属性才会被传递给着色器
    glEnableVertexAttribArray(0);

    // 解绑 VBO（可选，但推荐）
    // 注意：不要解绑 EBO！因为 EBO 存储在 VAO 中
    // 如果解绑 EBO，VAO 会丢失对它的引用
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    
    // 解绑 VAO（可选，但推荐）
    // 这样可以防止意外修改 VAO
    glBindVertexArray(0); 

    // ========================================================================
    // 第六步：渲染循环（游戏循环）
    // ========================================================================
    // 这个循环会一直运行，直到窗口被关闭
    // 每帧都会：
    // 1. 处理输入
    // 2. 清屏
    // 3. 绘制图形
    // 4. 交换缓冲区（显示画面）
    // 5. 处理事件
    // ========================================================================
    while (!glfwWindowShouldClose(window))
    {
        // ----------------------------------------------------------------
        // 6.1 处理输入
        // ----------------------------------------------------------------
        // 检查用户的键盘、鼠标输入
        // 例如：检查是否按了 ESC 键
        processInput(window);

        // ----------------------------------------------------------------
        // 6.2 清屏
        // ----------------------------------------------------------------
        // glClearColor: 设置清屏时使用的颜色（RGBA 格式）
        // 参数：(R, G, B, A)
        //   - 0.2f, 0.3f, 0.3f: RGB 颜色值（0.0-1.0），深青色
        //   - 1.0f: Alpha 透明度（1.0 = 完全不透明）
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        
        // glClear: 清空指定的缓冲区
        // GL_COLOR_BUFFER_BIT: 清空颜色缓冲区（就是清屏）
        // 这会把整个窗口填充为上面设置的颜色（深青色）
        glClear(GL_COLOR_BUFFER_BIT);

        // ----------------------------------------------------------------
        // 6.3 绘制图形
        // ----------------------------------------------------------------
        // glUseProgram: 激活着色器程序
        // 之后的所有绘制操作都会使用这个着色器程序
        glUseProgram(shaderProgram);
        
        // glBindVertexArray: 绑定 VAO
        // 这会自动绑定相关的 VBO 和 EBO
        glBindVertexArray(VAO);
        
        // glDrawElements: 使用索引绘制
        // 参数：
        //   - GL_TRIANGLES: 绘制模式（绘制三角形）
        //   - 6: 要绘制的索引数量（2 个三角形 × 3 个顶点 = 6）
        //   - GL_UNSIGNED_INT: 索引数据类型（无符号整数）
        //   - 0: 索引数组的偏移量（从索引 0 开始）
        // 
        // 这个函数会：
        // 1. 从 EBO 中读取索引
        // 2. 根据索引从 VBO 中获取顶点数据
        // 3. 将顶点数据传递给顶点着色器
        // 4. 顶点着色器处理顶点位置
        // 5. 光栅化（将三角形转换为像素）
        // 6. 片段着色器处理每个像素的颜色
        // 7. 最终显示在屏幕上
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // ----------------------------------------------------------------
        // 6.4 交换缓冲区和处理事件
        // ----------------------------------------------------------------
        // glfwSwapBuffers: 交换前后缓冲区
        // 解释：
        //   - 双缓冲机制：有两个缓冲区（前缓冲区和后缓冲区）
        //   - 我们在后缓冲区绘制画面
        //   - 绘制完成后，交换两个缓冲区，用户就能看到新画面
        //   - 这样可以避免画面闪烁
        glfwSwapBuffers(window);
        
        // glfwPollEvents: 处理窗口事件
        // 例如：窗口大小改变、鼠标移动、键盘输入等
        // 必须每帧调用，否则窗口会无响应
        glfwPollEvents();
    }

    // ========================================================================
    // 第七步：清理资源
    // ========================================================================
    // 程序结束前，应该释放所有 OpenGL 资源
    // 这是良好的编程习惯，确保资源被正确释放
    // ========================================================================
    
    // 删除 VAO（顶点数组对象）
    glDeleteVertexArrays(1, &VAO);
    
    // 删除 VBO（顶点缓冲区对象）
    glDeleteBuffers(1, &VBO);
    
    // 删除 EBO（索引缓冲区对象）
    glDeleteBuffers(1, &EBO);
    
    // 删除着色器程序
    glDeleteProgram(shaderProgram);

    // 清理 GLFW 资源
    glfwTerminate();
    
    // 程序正常退出
    return 0;
}

