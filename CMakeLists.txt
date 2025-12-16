cmake_minimum_required(VERSION 3.15)
project(OpenGLLearning)

set(CMAKE_CXX_STANDARD 20)

# 添加可执行文件（包含 GLAD 源文件）
add_executable(OpenGLLearning
        engine/src/main.cpp
)

