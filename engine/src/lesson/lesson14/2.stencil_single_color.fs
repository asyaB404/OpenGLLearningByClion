#version 330 core
out vec4 FragColor;                     // 输出：最终片段颜色

void main()
{
    // 单色输出（用于轮廓效果）
    FragColor = vec4(0.04f, 0.28f, 0.26f, 1.0f);  // 青色轮廓
}

