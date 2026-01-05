#version 330 core
out vec4 FragColor;                     // 输出：最终片段颜色

in vec3 Normal;                         // 输入：法线向量（从顶点着色器）
in vec3 FragPos;                        // 输入：片段位置（世界空间）

uniform vec3 objectColor;               // 物体颜色
uniform vec3 lightColor;                 // 光源颜色
uniform vec3 lightPos;                   // 光源位置（世界空间）
uniform vec3 viewPos;                    // 相机位置（世界空间）

void main()
{
    // 环境光（Ambient）
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射（Diffuse）
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 镜面反射（Specular）
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // 最终颜色 = (环境光 + 漫反射 + 镜面反射) × 物体颜色
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}

