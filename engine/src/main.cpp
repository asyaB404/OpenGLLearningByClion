// ============================================================================
// OpenGL 学习项目 - 主入口
// ============================================================================
// 这个文件是程序的主入口，用于选择运行哪个 lesson
// ============================================================================

#include <iostream>

#include "lesson/test/test.h"

// 声明各个 lesson 的主函数
extern int lesson1_main();
extern int lesson2_main();
extern int lesson3_1_main();
extern int lesson4_1_main();

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
    std::cout << "0. 测试\n";
    std::cout << "========================================\n";
    std::cout << "请选择 (0-4): ";
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    int choice = -1;
    
    while (choice != 0) {
        showMenu();
        std::cin >> choice;
        
        // 清除输入缓冲区
        std::cin.ignore();
        
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
