// ============================================================================
// Model 类 - 3D 模型加载和管理
// ============================================================================
// 这个类使用 Assimp 库加载 3D 模型文件（支持多种格式）
// 可以加载模型的网格、材质和纹理
// ============================================================================

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <cstring>  // for strcmp

// ============================================================================
// 辅助函数：从文件加载纹理
// ============================================================================
static unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// ============================================================================
// Model 类
// ============================================================================
class Model 
{
public:
    // 模型数据
    std::vector<Texture> textures_loaded;  // 存储所有已加载的纹理，优化以确保纹理不会加载多次
    std::vector<Mesh>    meshes;           // 所有网格
    std::string directory;                 // 模型文件所在目录
    bool gammaCorrection;                  // 是否进行伽马校正

    // ========================================================================
    // 构造函数，期望一个 3D 模型文件的路径
    // ========================================================================
    Model(std::string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // ========================================================================
    // 绘制模型，从而绘制其所有网格
    // ========================================================================
    void Draw(Shader &shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
    
private:
    // ========================================================================
    // 从文件加载模型，支持 ASSIMP 扩展名，并将生成的网格存储在 meshes 向量中
    // ========================================================================
    void loadModel(std::string const &path)
    {
        // 通过 ASSIMP 读取文件
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, 
            aiProcess_Triangulate |           // 三角化
            aiProcess_GenSmoothNormals |     // 生成平滑法线
            aiProcess_FlipUVs |                // 翻转 UV（OpenGL 需要）
            aiProcess_CalcTangentSpace);       // 计算切线空间
        
        // 检查错误
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        
        // 检索文件路径的目录路径
        directory = path.substr(0, path.find_last_of('/'));

        // 递归处理 ASSIMP 的根节点
        processNode(scene->mRootNode, scene);
    }

    // ========================================================================
    // 以递归方式处理节点。处理位于节点的每个单独网格，并对其子节点（如果有）重复此过程
    // ========================================================================
    void processNode(aiNode *node, const aiScene *scene)
    {
        // 处理位于当前节点的每个网格
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // 节点对象只包含索引来索引场景中的实际对象
            // 场景包含所有数据，节点只是用来保持组织（如节点之间的关系）
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        
        // 处理完所有网格（如果有）后，我们递归处理每个子节点
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    // ========================================================================
    // 处理网格
    // ========================================================================
    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        // 要填充的数据
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // 遍历网格的每个顶点
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // 我们声明一个占位符向量，因为 assimp 使用自己的向量类，不能直接转换为 glm 的 vec3 类，所以我们先将数据传输到这个占位符 glm::vec3
            
            // 位置
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            
            // 法线
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            
            // 纹理坐标
            if(mesh->mTextureCoords[0]) // 网格是否包含纹理坐标？
            {
                glm::vec2 vec;
                // 一个顶点最多可以包含 8 个不同的纹理坐标。因此我们假设我们不会
                // 使用一个顶点可以有多个纹理坐标的模型，所以我们总是取第一组（0）
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                
                // 切线
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                
                // 副切线
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        
        // 现在遍历网格的每个面（面是网格的三角形）并检索相应的顶点索引
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // 检索面的所有索引并将它们存储在索引向量中
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }
        
        // 处理材质
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // 我们假设着色器中采样器名称的约定。每个漫反射纹理应该命名为
        // 'texture_diffuseN'，其中 N 是从 1 到 MAX_SAMPLER_NUMBER 的连续数字
        // 其他纹理也适用相同的规则，如下列表总结：
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. 漫反射贴图
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        
        // 2. 镜面反射贴图
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        
        // 3. 法线贴图
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        
        // 4. 高度贴图
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        // 返回从提取的网格数据创建的网格对象
        return Mesh(vertices, indices, textures);
    }

    // ========================================================================
    // 检查给定类型的所有材质纹理，如果尚未加载则加载纹理
    // 所需信息作为 Texture 结构返回
    // ========================================================================
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            
            // 检查纹理是否之前已加载，如果是，继续下一次迭代：跳过加载新纹理
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // 已加载具有相同文件路径的纹理，继续下一个（优化）
                    break;
                }
            }
            if(!skip)
            {   // 如果纹理尚未加载，则加载它
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // 将其存储为整个模型已加载的纹理，以确保我们不会不必要地加载重复的纹理
            }
        }
        return textures;
    }
};

