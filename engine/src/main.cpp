// ============================================================================
// OpenGL 学习项目 - 主入口
// ============================================================================
// 这个文件是程序的主入口，用于选择运行哪个 lesson
// ============================================================================

#include <iostream>
#include <string>
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
extern int lesson6_1_oop_main();
extern int lesson7_1_main();
extern int lesson8_1_main();
extern int lesson8_2_main();
extern int lesson9_1_main();
extern int lesson10_1_main();
extern int lesson11_1_main();
extern int lesson11_2_main();
extern int lesson11_3_main();
extern int lesson12_1_main();
extern int lesson12_2_main();
extern int lesson12_3_main();
extern int lesson13_1_main();
extern int lesson13_2_main();
extern int lesson14_1_main();
extern int lesson15_1_main();
extern int lesson16_1_main();

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
    std::cout << "6-1. Lesson 6 OOP - 相机系统（面向对象版本）\n";
    std::cout << "7. Lesson 7 - 基础光照（Basic Lighting）\n";
    std::cout << "8. Lesson 8 - 基础光照（Phong 模型）\n";
    std::cout << "8-2. Lesson 8.2 - 基础光照（Phong 模型 + 动态光源）\n";
    std::cout << "9. Lesson 9 - 材质系统（Materials）\n";
    std::cout << "10. Lesson 10 - 光照贴图（Lighting Maps）\n";
    std::cout << "11. Lesson 11 - 方向光（Directional Light）\n";
    std::cout << "11-2. Lesson 11.2 - 点光源（Point Light）\n";
    std::cout << "11-3. Lesson 11.3 - 聚光灯（Spotlight）\n";
    std::cout << "12. Lesson 12 - 模型加载（Model Loading）\n";
    std::cout << "12-2. Lesson 12.2 - 模型加载 + 点光源\n";
    std::cout << "12-3. Lesson 12.3 - 模型加载 + 平行光\n";
    std::cout << "13-1. Lesson 13.1 - 深度测试（Depth Testing）\n";
    std::cout << "13-2. Lesson 13.2 - 深度缓冲可视化（Depth Buffer Visualization）\n";
    std::cout << "14. Lesson 14 - 模板缓冲轮廓效果（Stencil Buffer Outline）\n";
    std::cout << "15. Lesson 15 - 混合透明纹理（Blending Transparent Textures）\n";
    std::cout << "16. Lesson 16 - 帧缓冲和后期处理（Framebuffers & Post-processing）\n";
    std::cout << "0. 测试\n";
    std::cout << "========================================\n";
    std::cout << "输入 q 退出";
}

// ============================================================================
// 获取用户输入（支持 ESC 键检测和字符串输入）
// ============================================================================
std::string getUserInput() {
    std::string input;
    std::getline(std::cin, input);
    return input;
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    int choice = -1;
    
    while (true) {
        showMenu();
        
        // 获取用户输入
        std::string input = getUserInput();
        
        // 去除首尾空格
        while (!input.empty() && (input.front() == ' ' || input.front() == '\t')) {
            input.erase(0, 1);
        }
        while (!input.empty() && (input.back() == ' ' || input.back() == '\t')) {
            input.pop_back();
        }
        
        // 检测退出命令
        if (input == "q" || input == "Q" || input == "quit" || input == "exit") {
            std::cout << "\n退出程序。\n" << std::endl;
            break;
        }
        
        // 检测特殊输入
        if (input == "6-1") {
            std::cout << "\n>>> 运行 Lesson 6.1 OOP...\n" << std::endl;
            lesson6_1_oop_main();
            continue;
        }
        if (input == "8-2") {
            std::cout << "\n>>> 运行 Lesson 8.2...\n" << std::endl;
            lesson8_2_main();
            continue;
        }
        if (input == "11-2") {
            std::cout << "\n>>> 运行 Lesson 11.2...\n" << std::endl;
            lesson11_2_main();
            continue;
        }
        if (input == "11-3") {
            std::cout << "\n>>> 运行 Lesson 11.3...\n" << std::endl;
            lesson11_3_main();
            continue;
        }
        if (input == "12-2") {
            std::cout << "\n>>> 运行 Lesson 12.2...\n" << std::endl;
            lesson12_2_main();
            continue;
        }
        if (input == "12-3") {
            std::cout << "\n>>> 运行 Lesson 12.3...\n" << std::endl;
            lesson12_3_main();
            continue;
        }
        if (input == "13-1") {
            std::cout << "\n>>> 运行 Lesson 13.1...\n" << std::endl;
            lesson13_1_main();
            continue;
        }
        if (input == "13-2") {
            std::cout << "\n>>> 运行 Lesson 13.2...\n" << std::endl;
            lesson13_2_main();
            continue;
        }
        
        // 将字符串转换为数字
        try {
            choice = std::stoi(input);
        } catch (...) {
            std::cout << "\n无效的选择，请重新输入。\n" << std::endl;
            continue;
        }
        
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
                
            case 7:
                std::cout << "\n>>> 运行 Lesson 7.1...\n" << std::endl;
                lesson7_1_main();
                break;
                
            case 8:
                std::cout << "\n>>> 运行 Lesson 8.1...\n" << std::endl;
                lesson8_1_main();
                break;
                
            case 9:
                std::cout << "\n>>> 运行 Lesson 9.1...\n" << std::endl;
                lesson9_1_main();
                break;
                
            case 10:
                std::cout << "\n>>> 运行 Lesson 10.1...\n" << std::endl;
                lesson10_1_main();
                break;
                
            case 11:
                std::cout << "\n>>> 运行 Lesson 11.1...\n" << std::endl;
                lesson11_1_main();
                break;
                
            case 12:
                std::cout << "\n>>> 运行 Lesson 12.1...\n" << std::endl;
                lesson12_1_main();
                break;
                
            case 14:
                std::cout << "\n>>> 运行 Lesson 14.1...\n" << std::endl;
                lesson14_1_main();
                break;
                
            case 15:
                std::cout << "\n>>> 运行 Lesson 15...\n" << std::endl;
                lesson15_1_main();
                break;
                
            case 16:
                std::cout << "\n>>> 运行 Lesson 16...\n" << std::endl;
                lesson16_1_main();
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
