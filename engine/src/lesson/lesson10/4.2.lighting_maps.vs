#version 330 core
layout (location = 0) in vec3 aPos;      // 输入：顶点位置
layout (location = 1) in vec3 aNormal;  // 输入：法线向量
layout (location = 2) in vec2 aTexCoord;// 输入：纹理坐标

out vec3 Normal;                        // 输出：法线向量（传递给片段着色器）
out vec3 FragPos;                       // 输出：片段位置（世界空间）
out vec2 TexCoord;                      // 输出：纹理坐标（传递给片段着色器）

uniform mat4 model;                     // 模型矩阵
uniform mat4 view;                      // 视图矩阵
uniform mat4 projection;                // 投影矩阵

void main()
{
    // 计算片段在世界空间中的位置
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // 将法线向量从局部空间变换到世界空间
    // 注意：法线矩阵是模型矩阵的逆矩阵的转置（这里简化处理）
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // 传递纹理坐标
    TexCoord = aTexCoord;
    
    // 应用模型、视图和投影变换
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

