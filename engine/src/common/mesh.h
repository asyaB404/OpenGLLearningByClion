// ============================================================================
// Mesh 类 - 网格数据管理
// ============================================================================
// 这个类封装了网格的顶点数据、索引数据和纹理数据
// 提供了渲染网格的功能
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <cstddef>  // for offsetof
#include "shader.h"

#define MAX_BONE_INFLUENCE 4

// ============================================================================
// Vertex 结构体 - 顶点数据
// ============================================================================
struct Vertex {
    glm::vec3 Position;    // 位置
    glm::vec3 Normal;      // 法线
    glm::vec2 TexCoords;   // 纹理坐标
    glm::vec3 Tangent;     // 切线
    glm::vec3 Bitangent;   // 副切线
    int m_BoneIDs[MAX_BONE_INFLUENCE];  // 骨骼索引（用于骨骼动画）
    float m_Weights[MAX_BONE_INFLUENCE]; // 骨骼权重（用于骨骼动画）
};

// ============================================================================
// Texture 结构体 - 纹理数据
// ============================================================================
struct Texture {
    unsigned int id;       // 纹理 ID
    std::string type;      // 纹理类型（如 "texture_diffuse", "texture_specular"）
    std::string path;      // 纹理文件路径
};

// ============================================================================
// Mesh 类
// ============================================================================
class Mesh {
public:
    // 网格数据
    std::vector<Vertex>       vertices;  // 顶点数据
    std::vector<unsigned int> indices;   // 索引数据
    std::vector<Texture>      textures;  // 纹理数据
    unsigned int VAO;                    // 顶点数组对象

    // ========================================================================
    // 构造函数
    // ========================================================================
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // 设置顶点缓冲区和属性指针
        setupMesh();
    }

    // ========================================================================
    // 渲染网格
    // ========================================================================
    void Draw(Shader &shader) 
    {
        // 绑定适当的纹理
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // 在绑定之前激活相应的纹理单元
            
            // 检索纹理编号（diffuse_textureN 中的 N）
            std::string number;
            std::string name = textures[i].type;
            
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);
            else if(name == "texture_normal")
                number = std::to_string(normalNr++);
            else if(name == "texture_height")
                number = std::to_string(heightNr++);

            // 现在将采样器设置为正确的纹理单元
            shader.setInt((name + number).c_str(), i);
            // 最后绑定纹理
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        
        // 绘制网格
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // 配置完成后，将一切设置回默认值是一个好习惯
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // 渲染数据
    unsigned int VBO, EBO;

    // ========================================================================
    // 初始化所有缓冲区对象/数组
    // ========================================================================
    void setupMesh()
    {
        // 创建缓冲区/数组
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        
        // 将数据加载到顶点缓冲区
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // 结构体的一个很好的特性是它们的内存布局对所有项目都是顺序的
        // 效果是我们可以简单地传递一个指向结构的指针，它完美地转换为 glm::vec3/2 数组
        // 这又转换为 3/2 个浮点数，再转换为字节数组
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 设置顶点属性指针
        // 顶点位置
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        
        // 顶点法线
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        
        // 顶点纹理坐标
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        
        // 顶点切线
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        
        // 顶点副切线
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        
        // 骨骼 ID
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // 骨骼权重
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        
        glBindVertexArray(0);
    }
};

