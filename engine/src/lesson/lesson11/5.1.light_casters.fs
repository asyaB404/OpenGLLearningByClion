#version 330 core
out vec4 FragColor;                     // 输出：最终片段颜色

in vec3 Normal;                         // 输入：法线向量（从顶点着色器）
in vec3 FragPos;                        // 输入：片段位置（世界空间）
in vec2 TexCoord;                       // 输入：纹理坐标（从顶点着色器）

uniform vec3 viewPos;                   // 相机位置（世界空间）

// 材质属性（使用纹理贴图）
struct Material {
    sampler2D diffuse;                  // 漫反射贴图
    sampler2D specular;                 // 镜面反射贴图
    float shininess;                    // 高光指数（Shininess）
};

// 方向光（Directional Light）属性
struct Light {
    vec3 direction;                     // 光源方向（世界空间）
    vec3 ambient;                       // 环境光颜色
    vec3 diffuse;                       // 漫反射颜色
    vec3 specular;                      // 镜面反射颜色
};

uniform Material material;              // 材质
uniform Light light;                    // 光源

void main()
{
    // 环境光（Ambient）
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

    // 漫反射（Diffuse）
    vec3 norm = normalize(Normal);
    // 方向光：光源方向是固定的，不需要计算（直接使用 light.direction）
    vec3 lightDir = normalize(-light.direction);  // 注意：取反，因为 direction 指向光源
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoord)));

    // 镜面反射（Specular）
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));

    // 最终颜色 = 环境光 + 漫反射 + 镜面反射
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}

