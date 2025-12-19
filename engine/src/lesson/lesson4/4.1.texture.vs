#version 330 core
layout (location = 0) in vec3 aPos;        // 输入：顶点位置
layout (location = 1) in vec3 aColor;      // 输入：顶点颜色
layout (location = 2) in vec2 aTexCoord;   // 输入：纹理坐标

out vec3 ourColor;                         // 输出：传递给片段着色器的颜色
out vec2 TexCoord;                         // 输出：传递给片段着色器的纹理坐标

void main()
{
    gl_Position = vec4(aPos, 1.0);        // 设置顶点位置
    ourColor = aColor;                     // 将颜色传递给片段着色器
    TexCoord = aTexCoord;                  // 将纹理坐标传递给片段着色器
}

