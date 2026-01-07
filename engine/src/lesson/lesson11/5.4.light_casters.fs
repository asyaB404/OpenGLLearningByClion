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

// 聚光灯（Spotlight）属性
struct Light {
    vec3 position;                      // 光源位置（世界空间）
    vec3 direction;                     // 光源方向（世界空间）
    float cutOff;                       // 内角余弦值（内圆锥）
    float outerCutOff;                  // 外角余弦值（外圆锥）
    
    vec3 ambient;                       // 环境光颜色
    vec3 diffuse;                       // 漫反射颜色
    vec3 specular;                      // 镜面反射颜色
    
    // 衰减系数（Attenuation）
    float constant;                     // 常数项（通常为 1.0）
    float linear;                        // 线性项
    float quadratic;                     // 二次项
};

uniform Material material;              // 材质
uniform Light light;                    // 光源

void main()
{
    // 环境光（Ambient）
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

    // 漫反射（Diffuse）
    vec3 norm = normalize(Normal);
    // 计算从片段到光源的方向
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoord)));

    // 镜面反射（Specular）
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));

    // 计算聚光灯强度（Spotlight Intensity）
    // 计算片段到光源的方向与光源方向的夹角
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    // 应用聚光灯强度
    diffuse *= intensity;
    specular *= intensity;

    // 计算距离衰减（Distance Attenuation）
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // 应用衰减（环境光不受衰减影响，因为它模拟间接光照）
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // 最终颜色 = 环境光 + 漫反射 + 镜面反射
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}

