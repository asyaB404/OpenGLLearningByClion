#version 330 core
out vec4 FragColor;                     // 输出：最终片段颜色

uniform vec3 objectColor;               // 物体颜色

void main()
{
    FragColor = vec4(objectColor, 1.0);
}

