// ============================================================================
// Shader 类 - 着色器管理工具
// ============================================================================
// 这个类封装了着色器的编译、链接和使用功能
// 可以方便地从文件加载顶点着色器和片段着色器，并管理着色器程序
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

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
    // 构造函数（带几何着色器）：从文件路径加载并编译顶点、几何、片段着色器
    // ========================================================================
    // 参数：
    //   - vertexPath:   顶点着色器文件路径
    //   - fragmentPath: 片段着色器文件路径
    //   - geometryPath: 几何着色器文件路径（可选，传 nullptr 则等价于双参数构造函数）
    // ========================================================================
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
    {
        std::string vertexCode, fragmentCode, geometryCode;
        std::ifstream vShaderFile, fShaderFile, gShaderFile;
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vStream, fStream;
            vStream << vShaderFile.rdbuf();
            fStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vStream.str();
            fragmentCode = fStream.str();
            if (geometryPath && geometryPath[0] != '\0') {
                gShaderFile.open(geometryPath);
                std::stringstream gStream;
                gStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gStream.str();
            }
        }
        catch (std::ifstream::failure& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vCode = vertexCode.c_str();
        const char* fCode = fragmentCode.c_str();
        unsigned int vertex, fragment;
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        unsigned int geometry = 0;
        if (!geometryCode.empty()) {
            const char* gCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
            glAttachShader(ID, geometry);
        }
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometry) glDeleteShader(geometry);
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

    // 设置 vec2 类型的 uniform 变量
    void setVec2(const std::string &name, const glm::vec2 &value) const
    { 
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec2(const std::string &name, float x, float y) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
    }

    // 设置 vec3 类型的 uniform 变量
    void setVec3(const std::string &name, const glm::vec3 &value) const
    { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec3(const std::string &name, float x, float y, float z) const
    { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
    }

    // 设置 vec4 类型的 uniform 变量
    void setVec4(const std::string &name, const glm::vec4 &value) const
    { 
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
    }
    void setVec4(const std::string &name, float x, float y, float z, float w) const
    { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
    }

    // 设置 mat2 类型的 uniform 变量
    void setMat2(const std::string &name, const glm::mat2 &mat) const
    {
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    // 设置 mat3 类型的 uniform 变量
    void setMat3(const std::string &name, const glm::mat3 &mat) const
    {
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    // 设置 mat4 类型的 uniform 变量
    void setMat4(const std::string &name, const glm::mat4 &mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
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

