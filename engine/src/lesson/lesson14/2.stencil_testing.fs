#version 330 core
out vec4 FragColor;                     // 输出：最终片段颜色

in vec2 TexCoord;                       // 输入：纹理坐标（从顶点着色器）

uniform sampler2D texture1;             // 纹理采样器

void main()
{
    // 使用纹理颜色
    FragColor = texture(texture1, TexCoord);
}

