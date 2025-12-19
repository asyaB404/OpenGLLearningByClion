// ============================================================================
// Shader 类 - 着色器管理工具
// ============================================================================
// 这个类封装了着色器的编译、链接和使用功能
// 可以方便地从文件加载顶点着色器和片段着色器，并管理着色器程序
// ============================================================================

#pragma once

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// ============================================================================
// Shader 类
// ============================================================================
// 功能：
//   1. 从文件加载顶点着色器和片段着色器
//   2. 编译和链接着色器程序
//   3. 提供便捷的 uniform 变量设置方法
// ============================================================================
class Shader
{
public:
    unsigned int ID;  // 着色器程序的 OpenGL ID

    // ========================================================================
    // 构造函数：从文件路径加载并编译着色器
    // ========================================================================
    // 参数：
    //   - vertexPath:   顶点着色器文件路径
    //   - fragmentPath: 片段着色器文件路径
    // 
    // 功能：
    //   1. 读取顶点着色器和片段着色器文件
    //   2. 编译两个着色器
    //   3. 链接成着色器程序
    //   4. 检查编译和链接错误
    // ========================================================================
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        // 1. 从文件读取顶点着色器和片段着色器源码
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        
        // 确保 ifstream 对象可以抛出异常
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        
        try 
        {
            // 打开文件
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            
            // 读取文件内容到流中
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            
            // 关闭文件
            vShaderFile.close();
            fShaderFile.close();
            
            // 将流转换为字符串
            vertexCode   = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
        
        // 2. 编译着色器
        unsigned int vertex, fragment;
        
        // 编译顶点着色器
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        
        // 编译片段着色器
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        
        // 3. 创建着色器程序并链接
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        
        // 4. 删除着色器对象（已经链接到程序中，不再需要）
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // ========================================================================
    // 激活着色器程序
    // ========================================================================
    // 在渲染前调用此函数来使用这个着色器程序
    // ========================================================================
    void use() 
    { 
        glUseProgram(ID); 
    }

    // ========================================================================
    // Uniform 变量设置函数
    // ========================================================================
    // 这些函数用于在着色器中设置 uniform 变量的值
    // ========================================================================

    // 设置 bool 类型的 uniform 变量
    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }

    // 设置 int 类型的 uniform 变量
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }

    // 设置 float 类型的 uniform 变量
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }

private:
    // ========================================================================
    // 检查着色器编译/链接错误
    // ========================================================================
    // 参数：
    //   - shader: 着色器或程序 ID
    //   - type:   类型字符串（"VERTEX", "FRAGMENT", "PROGRAM"）
    // 
    // 功能：
    //   检查编译或链接是否成功，如果失败则输出错误信息
    // ========================================================================
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        
        if (type != "PROGRAM")
        {
            // 检查着色器编译状态
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            // 检查程序链接状态
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" 
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

