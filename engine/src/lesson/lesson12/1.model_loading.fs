#version 330 core
out vec4 FragColor;                     // 输出：最终片段颜色

in vec2 TexCoord;                       // 输入：纹理坐标（从顶点着色器）

// 材质纹理（支持多个纹理）
uniform sampler2D texture_diffuse1;     // 漫反射纹理 1
uniform sampler2D texture_specular1;    // 镜面反射纹理 1

void main()
{
    // 使用漫反射纹理作为基础颜色
    FragColor = texture(texture_diffuse1, TexCoord);
}

