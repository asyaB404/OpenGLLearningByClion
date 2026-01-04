#version 330 core
out vec4 FragColor;                       // 输出：最终片段颜色

in vec2 TexCoord;                         // 输入：从顶点着色器传来的纹理坐标

uniform sampler2D texture1;               // 纹理采样器 1
uniform sampler2D texture2;               // 纹理采样器 2

void main()
{
    // 混合两个纹理：texture1 占 80%，texture2 占 20%
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
}

