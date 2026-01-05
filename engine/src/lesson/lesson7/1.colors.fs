#version 330 core
out vec4 FragColor;                       // 输出：最终片段颜色

uniform vec3 objectColor;                 // 物体颜色
uniform vec3 lightColor;                  // 光源颜色

void main()
{
    // 简单的颜色计算：物体颜色 × 光源颜色
    FragColor = vec4(lightColor * objectColor, 1.0);
}

