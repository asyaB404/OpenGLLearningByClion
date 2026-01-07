#version 330 core
layout (location = 0) in vec3 aPos;      // 输入：顶点位置
layout (location = 1) in vec3 aNormal;  // 输入：法线向量
layout (location = 2) in vec2 aTexCoord;// 输入：纹理坐标

out vec2 TexCoord;                       // 输出：纹理坐标（传递给片段着色器）

uniform mat4 model;                     // 模型矩阵
uniform mat4 view;                      // 视图矩阵
uniform mat4 projection;                // 投影矩阵

void main()
{
    // 传递纹理坐标
    TexCoord = aTexCoord;
    
    // 应用模型、视图和投影变换
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

