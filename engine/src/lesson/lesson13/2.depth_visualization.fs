#version 330 core
out vec4 FragColor;                     // 输出：最终片段颜色

void main()
{
    // 使用 gl_FragCoord.z 获取当前片段的深度值
    // gl_FragCoord.z 的范围是 [0.0, 1.0]，其中：
    // - 0.0 表示最近的物体（near plane）
    // - 1.0 表示最远的物体（far plane）
    float depth = gl_FragCoord.z;
    
    // 将深度值可视化：近处为白色，远处为黑色
    FragColor = vec4(vec3(depth), 1.0);
}

