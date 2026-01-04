// ============================================================================
// OpenGL 学习项目 - 主入口
// ============================================================================
// 这个文件是程序的主入口，用于选择运行哪个 lesson
// ============================================================================

#include <iostream>
#include <cstdio>
#include <termios.h>
#include <unistd.h>

#include "lesson/test/test.h"

// 声明各个 lesson 的主函数
extern int lesson1_main();
extern int lesson2_main();
extern int lesson3_1_main();
extern int lesson4_1_main();
extern int lesson5_1_main();
extern int lesson6_1_main();

// ============================================================================
// 显示菜单
// ============================================================================
void showMenu() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "    OpenGL 学习项目 - 选择 Lesson\n";
    std::cout << "========================================\n";
    std::cout << "1. Lesson 1 - 创建窗口和基本渲染循环\n";
    std::cout << "2. Lesson 2 - 绘制第一个三角形\n";
    std::cout << "3. Lesson 3.1 - 顶点颜色（彩色三角形）\n";
    std::cout << "4. Lesson 4 - 纹理（Texture）\n";
    std::cout << "5. Lesson 5 - 坐标系统（Coordinate Systems）\n";
    std::cout << "6. Lesson 6 - 相机系统（Camera System）\n";
    std::cout << "0. 测试\n";
    std::cout << "========================================\n";
    std::cout << "请选择 (0-6, 按 ESC 或 q 退出): ";
}

// ============================================================================
// 获取单个字符输入（支持 ESC 键检测）
// ============================================================================
int getCharInput() {
    struct termios oldt, newt;
    int ch;
    
    // 保存当前终端设置
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    
    // 设置为原始模式（不缓冲，不回显）
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    // 读取一个字符
    ch = getchar();
    
    // 恢复终端设置
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    return ch;
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    int choice = -1;
    
    while (true) {
        showMenu();
        
        // 使用字符输入以支持 ESC 键检测
        int ch = getCharInput();
        
        // 检测 ESC 键（ASCII 码 27）或 'q'/'Q' 键
        if (ch == 27 || ch == 'q' || ch == 'Q') {
            std::cout << "\n退出程序。\n" << std::endl;
            break;
        }
        
        // 将字符转换为数字
        if (ch >= '0' && ch <= '6') {
            choice = ch - '0';
        } else {
            std::cout << "\n无效的选择，请重新输入。\n" << std::endl;
            continue;
        }
        
        // 清除输入缓冲区
        std::cin.ignore(10000, '\n');
        
        switch (choice) {
            case 1:
                std::cout << "\n>>> 运行 Lesson 1...\n" << std::endl;
                lesson1_main();
                break;
                
            case 2:
                std::cout << "\n>>> 运行 Lesson 2...\n" << std::endl;
                lesson2_main();
                break;
                
            case 3:
                std::cout << "\n>>> 运行 Lesson 3.1...\n" << std::endl;
                lesson3_1_main();
                break;
                
            case 4:
                std::cout << "\n>>> 运行 Lesson 4.1...\n" << std::endl;
                lesson4_1_main();
                break;
                
            case 5:
                std::cout << "\n>>> 运行 Lesson 5.1...\n" << std::endl;
                lesson5_1_main();
                break;
                
            case 6:
                std::cout << "\n>>> 运行 Lesson 6.1...\n" << std::endl;
                lesson6_1_main();
                break;
                
            case 0:
                test::testFunc();
                break;
                
            default:
                std::cout << "\n无效的选择，请重新输入。\n" << std::endl;
                break;
        }
    }
    
    return 0;
}
