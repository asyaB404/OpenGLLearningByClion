# Lesson 12: 模型加载总结

## 📚 目录
- [概述](#概述)
- [Lesson 12.1: 基础模型加载](#lesson-121-基础模型加载)
- [Lesson 12.2: 模型加载 + 点光源](#lesson-122-模型加载--点光源)
- [Lesson 12.3: 模型加载 + 平行光](#lesson-123-模型加载--平行光)
- [核心类详解](#核心类详解)
- [技术细节](#技术细节)
- [🌟 重点：有/无点光源的区别](#-重点有无点光源的区别)
- [光照模型对比](#光照模型对比)
- [使用指南](#使用指南)
- [常见问题](#常见问题)
- [总结](#总结)

---

## 概述

Lesson 12 介绍了如何使用 **Assimp 库**加载 3D 模型文件，并在模型上应用不同的光照效果。本课程分为三个部分：

1. **Lesson 12.1** - 基础模型加载（无光照，仅显示纹理）
2. **Lesson 12.2** - 模型加载 + 点光源（Point Light）
3. **Lesson 12.3** - 模型加载 + 平行光（Directional Light）

### 学习目标

- ✅ 理解如何使用 Assimp 库加载 3D 模型
- ✅ 掌握 Mesh 类和 Model 类的实现
- ✅ 理解模型数据的结构（顶点、索引、纹理）
- ✅ 学会在加载的模型上应用光照效果
- ✅ 对比不同光照类型在模型上的效果

---

## Lesson 12.1: 基础模型加载

### 功能特点

- **模型加载**：使用 Assimp 库加载 3D 模型文件（支持 40+ 种格式）
- **纹理显示**：显示模型的漫反射纹理
- **无光照**：仅显示纹理颜色，不进行光照计算

### 着色器

**顶点着色器** (`1.model_loading.vs`)：
```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoord = aTexCoord;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

**片段着色器** (`1.model_loading.fs`)：
```glsl
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoord);
}
```

### 代码结构

```cpp
class Lesson12Application : public CameraApplication
{
    // 初始化：加载模型和着色器
    virtual void OnInitialize() override;
    
    // 渲染：绘制模型
    virtual void OnRender() override;
    
private:
    Shader* m_shader;
    Model* m_model;
};
```

### 关键代码

```cpp
// 加载模型
std::string modelPath = std::string(PROJECT_ROOT) + "/engine/assets/models/backpack/backpack.obj";
m_model = new Model(modelPath);

// 渲染模型
m_shader->use();
m_shader->setMat4("projection", projection);
m_shader->setMat4("view", view);
m_shader->setMat4("model", model);
m_model->Draw(*m_shader);
```

---

## Lesson 12.2: 模型加载 + 点光源

### 功能特点

- **模型加载**：加载 3D 模型
- **点光源**：添加点光源光照效果
- **距离衰减**：光线强度随距离衰减
- **Phong 光照模型**：环境光 + 漫反射 + 镜面反射

### 着色器

**顶点着色器** (`2.model_loading_point_light.vs`)：
```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 Normal;      // 输出法线（用于光照计算）
out vec3 FragPos;     // 输出片段位置（用于光照计算）
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

**片段着色器** (`2.model_loading_point_light.fs`)：
```glsl
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

struct Material {
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;

void main()
{
    // 环境光
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoord));
    
    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * vec3(texture(texture_diffuse1, TexCoord)));
    
    // 镜面反射
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(texture_specular1, TexCoord)));
    
    // 距离衰减
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

### 关键代码

```cpp
// 设置点光源属性
m_shader->setVec3("light.position", m_lightPos);
m_shader->setVec3("viewPos", m_camera.Position);

// 光源属性
m_shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
m_shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
m_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

// 衰减系数
m_shader->setFloat("light.constant", 1.0f);
m_shader->setFloat("light.linear", 0.09f);
m_shader->setFloat("light.quadratic", 0.032f);

// 材质属性
m_shader->setFloat("material.shininess", 32.0f);
```

### 点光源特点

- **有位置**：光源在世界空间中的具体位置
- **距离衰减**：光线强度随距离衰减
- **适合场景**：灯泡、火把等局部光源

---

## Lesson 12.3: 模型加载 + 平行光

### 功能特点

- **模型加载**：加载 3D 模型
- **平行光**：添加平行光光照效果
- **无距离衰减**：所有片段使用相同的光照方向
- **Phong 光照模型**：环境光 + 漫反射 + 镜面反射

### 着色器

**顶点着色器** (`3.model_loading_directional_light.vs`)：
- 与点光源版本相同，需要输出法线和片段位置

**片段着色器** (`3.model_loading_directional_light.fs`)：
```glsl
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

struct Material {
    float shininess;
};

struct Light {
    vec3 direction;    // 平行光使用方向，而不是位置
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main()
{
    // 环境光
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoord));
    
    // 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);  // 取反
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * vec3(texture(texture_diffuse1, TexCoord)));
    
    // 镜面反射
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(texture_specular1, TexCoord)));
    
    // 无距离衰减
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

### 关键代码

```cpp
// 设置平行光属性
m_shader->setVec3("light.direction", -0.2f, -1.0f, -0.3f);
m_shader->setVec3("viewPos", m_camera.Position);

// 光源属性
m_shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
m_shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
m_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

// 材质属性
m_shader->setFloat("material.shininess", 32.0f);
```

### 平行光特点

- **无位置**：来自无限远
- **固定方向**：所有片段使用相同的光照方向
- **无距离衰减**：距离不影响光照强度
- **适合场景**：太阳光等全局光源

---

## 核心类详解

### Mesh 类

**位置**：`engine/src/common/mesh.h`

**功能**：
- 封装单个网格的顶点数据、索引数据和纹理数据
- 管理 VAO、VBO、EBO
- 提供渲染方法

**数据结构**：

```cpp
struct Vertex {
    glm::vec3 Position;    // 位置
    glm::vec3 Normal;      // 法线
    glm::vec2 TexCoords;   // 纹理坐标
    glm::vec3 Tangent;     // 切线
    glm::vec3 Bitangent;   // 副切线
    int m_BoneIDs[MAX_BONE_INFLUENCE];  // 骨骼索引（用于动画）
    float m_Weights[MAX_BONE_INFLUENCE]; // 骨骼权重（用于动画）
};

struct Texture {
    unsigned int id;       // 纹理 ID
    std::string type;      // 纹理类型（"texture_diffuse", "texture_specular" 等）
    std::string path;      // 纹理文件路径
};

class Mesh {
    std::vector<Vertex>       vertices;  // 顶点数据
    std::vector<unsigned int>  indices;   // 索引数据
    std::vector<Texture>      textures;  // 纹理数据
    unsigned int VAO;                    // 顶点数组对象
    
    void Draw(Shader &shader);          // 渲染网格
};
```

**关键方法**：

1. **构造函数**：
   - 接收顶点、索引和纹理数据
   - 调用 `setupMesh()` 初始化 OpenGL 缓冲区

2. **setupMesh()**：
   - 创建 VAO、VBO、EBO
   - 设置顶点属性指针（位置、法线、纹理坐标、切线、副切线、骨骼数据）

3. **Draw()**：
   - 绑定纹理（根据类型自动命名：`texture_diffuse1`, `texture_specular1` 等）
   - 设置着色器 uniform 变量
   - 绘制网格

### Model 类

**位置**：`engine/src/common/model.h`

**功能**：
- 使用 Assimp 库加载 3D 模型文件
- 解析模型的网格、材质和纹理
- 管理多个 Mesh 对象

**关键方法**：

1. **构造函数**：
   ```cpp
   Model(std::string const &path, bool gamma = false)
   ```
   - 接收模型文件路径
   - 调用 `loadModel()` 加载模型

2. **loadModel()**：
   - 使用 Assimp 读取模型文件
   - 设置后处理标志：
     - `aiProcess_Triangulate`：三角化
     - `aiProcess_GenSmoothNormals`：生成平滑法线
     - `aiProcess_FlipUVs`：翻转 UV（OpenGL 需要）
     - `aiProcess_CalcTangentSpace`：计算切线空间
   - 调用 `processNode()` 递归处理场景节点

3. **processNode()**：
   - 递归处理场景图节点
   - 处理每个节点的网格
   - 递归处理子节点

4. **processMesh()**：
   - 提取顶点数据（位置、法线、纹理坐标、切线、副切线）
   - 提取索引数据
   - 加载材质纹理（漫反射、镜面反射、法线、高度贴图）
   - 返回 Mesh 对象

5. **loadMaterialTextures()**：
   - 加载材质纹理
   - 纹理去重优化（避免重复加载相同纹理）
   - 返回纹理列表

6. **Draw()**：
   - 遍历所有网格并渲染

---

## 技术细节

### Assimp 库

**功能**：
- 支持 40+ 种 3D 模型格式
- 自动处理模型数据转换
- 提取网格、材质、纹理、动画等数据

**支持的格式**：
- OBJ, FBX, DAE, 3DS, BLEND, X, MD2, MD3, MD5, B3D, IQM, MS3D, ASE, PLY, LWO, LWS, LXO, STL, AC, DXF, HMP, SMD, MDL, MDC, MDL, NFF, OFF, RAW, SCN, BVH, IRRMESH, IRR, Q3D, Q3BSP, BSP, NDO, COB, TER, CSM, Q3D, B3D, XGL, FBX, GLTF, GLB 等

**后处理标志**：

| 标志 | 说明 |
|------|------|
| `aiProcess_Triangulate` | 将所有图元转换为三角形 |
| `aiProcess_GenSmoothNormals` | 生成平滑法线 |
| `aiProcess_FlipUVs` | 翻转 UV 坐标（OpenGL 需要） |
| `aiProcess_CalcTangentSpace` | 计算切线和副切线（用于法线贴图） |
| `aiProcess_JoinIdenticalVertices` | 合并重复顶点 |
| `aiProcess_OptimizeMeshes` | 优化网格 |
| `aiProcess_OptimizeGraph` | 优化场景图 |

### 纹理命名约定

Mesh 类在渲染时会自动为纹理设置 uniform 变量：

- **漫反射纹理**：`texture_diffuse1`, `texture_diffuse2`, ...
- **镜面反射纹理**：`texture_specular1`, `texture_specular2`, ...
- **法线贴图**：`texture_normal1`, `texture_normal2`, ...
- **高度贴图**：`texture_height1`, `texture_height2`, ...

**注意**：在 GLSL 中，`sampler2D` 不能放在 struct 中，需要作为独立的 uniform 变量。

### 法线矩阵

在顶点着色器中，法线需要从局部空间变换到世界空间：

```glsl
Normal = mat3(transpose(inverse(model))) * aNormal;
```

**为什么需要法线矩阵？**

- 法线是方向向量，不应该被缩放影响
- 法线矩阵 = 模型矩阵的逆矩阵的转置
- 确保法线在非均匀缩放时仍然正确

### 顶点属性布局

Mesh 类设置的顶点属性：

| Location | 属性 | 类型 | 说明 |
|----------|------|------|------|
| 0 | Position | vec3 | 顶点位置 |
| 1 | Normal | vec3 | 法线向量 |
| 2 | TexCoords | vec2 | 纹理坐标 |
| 3 | Tangent | vec3 | 切线向量 |
| 4 | Bitangent | vec3 | 副切线向量 |
| 5 | m_BoneIDs | ivec4 | 骨骼索引（用于动画） |
| 6 | m_Weights | vec4 | 骨骼权重（用于动画） |

---

## 🌟 重点：有/无点光源的区别

这是 Lesson 12 中最重要的概念之一。理解有/无点光源的区别，可以帮助您更好地理解光照在 3D 渲染中的作用。

### 视觉对比

#### 无点光源（Lesson 12.1）

**视觉效果**：
- 📷 **平面感**：模型看起来是"平的"，没有立体感
- 🎨 **纯纹理**：只显示纹理本身的颜色，没有明暗变化
- 🔲 **无阴影**：所有面都是均匀的亮度
- 📐 **无深度感**：无法区分模型的凹凸和形状

**就像**：在完全均匀的灯光下看一张照片，所有地方都一样亮。

#### 有点光源（Lesson 12.2）

**视觉效果**：
- 🎯 **立体感**：模型有明显的明暗变化，看起来是 3D 的
- 💡 **光照效果**：面向光源的面更亮，背向光源的面更暗
- ✨ **高光反射**：在光滑表面上有亮点（镜面反射）
- 📏 **距离感**：距离光源近的地方更亮，远的地方更暗（距离衰减）

**就像**：在房间里点一盏灯，物体有明暗变化，看起来更真实。

### 代码对比

#### 1. 顶点着色器对比

**无点光源（Lesson 12.1）**：
```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;  // 虽然输入了法线，但没有使用
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoord;  // 只输出纹理坐标

void main()
{
    TexCoord = aTexCoord;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // 注意：没有计算法线和片段位置
}
```

**有点光源（Lesson 12.2）**：
```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;  // 使用法线
layout (location = 2) in vec2 aTexCoord;

out vec3 Normal;    // 输出法线（用于光照计算）
out vec3 FragPos;   // 输出片段位置（用于光照计算）
out vec2 TexCoord;

void main()
{
    // 计算片段在世界空间中的位置
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // 将法线从局部空间变换到世界空间
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

**关键区别**：
- ✅ **法线计算**：有点光源需要计算法线（用于漫反射和镜面反射）
- ✅ **片段位置**：有点光源需要计算片段在世界空间中的位置（用于计算光源方向）
- ✅ **法线矩阵**：需要将法线从局部空间变换到世界空间

#### 2. 片段着色器对比

**无点光源（Lesson 12.1）**：
```glsl
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture_diffuse1;

void main()
{
    // 直接使用纹理颜色，不做任何光照计算
    FragColor = texture(texture_diffuse1, TexCoord);
}
```

**有点光源（Lesson 12.2）**：
```glsl
#version 330 core
out vec4 FragColor;

in vec3 Normal;      // 需要法线
in vec3 FragPos;     // 需要片段位置
in vec2 TexCoord;

uniform vec3 viewPos;           // 需要相机位置
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

struct Material {
    float shininess;
};

struct Light {
    vec3 position;      // 光源位置
    vec3 ambient;       // 环境光
    vec3 diffuse;       // 漫反射
    vec3 specular;      // 镜面反射
    float constant;     // 衰减系数
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;

void main()
{
    // 1. 环境光（Ambient）
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoord));
    
    // 2. 漫反射（Diffuse）
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);  // 计算光源方向
    float diff = max(dot(norm, lightDir), 0.0);           // 计算漫反射强度
    vec3 diffuse = light.diffuse * (diff * vec3(texture(texture_diffuse1, TexCoord)));
    
    // 3. 镜面反射（Specular）
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(texture_specular1, TexCoord)));
    
    // 4. 距离衰减
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // 应用衰减
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    // 最终颜色 = 环境光 + 漫反射 + 镜面反射
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

**关键区别**：

| 特性 | 无点光源 | 有点光源 |
|------|---------|---------|
| **输入数据** | 只需要纹理坐标 | 需要法线、片段位置、相机位置 |
| **计算复杂度** | 简单（1 行代码） | 复杂（多步计算） |
| **光照计算** | ❌ 无 | ✅ 环境光 + 漫反射 + 镜面反射 |
| **距离衰减** | ❌ 无 | ✅ 有 |
| **高光效果** | ❌ 无 | ✅ 有（镜面反射） |
| **纹理使用** | 只用漫反射纹理 | 用漫反射和镜面反射纹理 |

#### 3. C++ 代码对比

**无点光源（Lesson 12.1）**：
```cpp
virtual void OnRender() override
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->use();
    
    // 只需要设置变换矩阵
    m_shader->setMat4("projection", projection);
    m_shader->setMat4("view", view);
    m_shader->setMat4("model", model);
    
    m_model->Draw(*m_shader);
}
```

**有点光源（Lesson 12.2）**：
```cpp
virtual void OnRender() override
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->use();
    
    // 1. 设置光源属性
    m_shader->setVec3("light.position", m_lightPos);
    m_shader->setVec3("viewPos", m_camera.Position);
    
    // 2. 设置光源颜色和强度
    m_shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    m_shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    m_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    
    // 3. 设置衰减系数
    m_shader->setFloat("light.constant", 1.0f);
    m_shader->setFloat("light.linear", 0.09f);
    m_shader->setFloat("light.quadratic", 0.032f);
    
    // 4. 设置材质属性
    m_shader->setFloat("material.shininess", 32.0f);
    
    // 5. 设置变换矩阵
    m_shader->setMat4("projection", projection);
    m_shader->setMat4("view", view);
    m_shader->setMat4("model", model);
    
    m_model->Draw(*m_shader);
}
```

**关键区别**：
- ✅ **光源设置**：有点光源需要设置光源位置、颜色、衰减系数
- ✅ **相机位置**：需要设置相机位置（用于镜面反射计算）
- ✅ **材质属性**：需要设置材质的高光指数（shininess）

### 光照计算详解

#### 无点光源的计算

```
最终颜色 = 纹理颜色
```

**非常简单**：直接使用纹理的颜色，不做任何处理。

#### 有点光源的计算

```
最终颜色 = 环境光 + 漫反射 + 镜面反射
```

**详细步骤**：

1. **环境光（Ambient）**：
   ```
   环境光 = 光源环境光颜色 × 纹理颜色
   ```
   - 模拟间接光照（物体被其他物体反射的光照亮）
   - 确保即使没有直接光照，物体也不会完全黑暗

2. **漫反射（Diffuse）**：
   ```
   光源方向 = normalize(光源位置 - 片段位置)
   漫反射强度 = max(dot(法线, 光源方向), 0.0)
   漫反射 = 光源漫反射颜色 × 漫反射强度 × 纹理颜色
   ```
   - 模拟光线从光源照射到物体表面的效果
   - 面向光源的面更亮，背向光源的面更暗
   - 使用点积计算法线和光源方向的夹角

3. **镜面反射（Specular）**：
   ```
   视线方向 = normalize(相机位置 - 片段位置)
   反射方向 = reflect(-光源方向, 法线)
   镜面反射强度 = pow(max(dot(视线方向, 反射方向), 0.0), 高光指数)
   镜面反射 = 光源镜面反射颜色 × 镜面反射强度 × 镜面反射纹理
   ```
   - 模拟光滑表面的高光反射
   - 在特定角度可以看到亮点
   - 使用镜面反射纹理控制高光的颜色和强度

4. **距离衰减（Attenuation）**：
   ```
   距离 = length(光源位置 - 片段位置)
   衰减系数 = 1.0 / (常数项 + 线性项 × 距离 + 二次项 × 距离²)
   最终光照 = (环境光 + 漫反射 + 镜面反射) × 衰减系数
   ```
   - 模拟真实世界中光线随距离衰减的效果
   - 距离光源越远，光线越弱
   - 使用二次函数模拟衰减

### 数学公式对比

#### 无点光源

```
FragColor = texture(texture_diffuse1, TexCoord)
```

**复杂度**：O(1) - 只是一个纹理采样操作

#### 有点光源

```
// 环境光
ambient = light.ambient × texture(texture_diffuse1, TexCoord)

// 漫反射
lightDir = normalize(light.position - FragPos)
diff = max(dot(normalize(Normal), lightDir), 0.0)
diffuse = light.diffuse × diff × texture(texture_diffuse1, TexCoord)

// 镜面反射
viewDir = normalize(viewPos - FragPos)
reflectDir = reflect(-lightDir, normalize(Normal))
spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess)
specular = light.specular × spec × texture(texture_specular1, TexCoord)

// 距离衰减
distance = length(light.position - FragPos)
attenuation = 1.0 / (light.constant + light.linear × distance + light.quadratic × distance²)

// 最终颜色
result = (ambient + diffuse + specular) × attenuation
FragColor = vec4(result, 1.0)
```

**复杂度**：O(1) - 但计算步骤更多，包括：
- 3 次向量归一化
- 2 次点积计算
- 1 次反射计算
- 1 次幂运算
- 1 次距离计算
- 1 次除法运算
- 多次向量运算

### 性能对比

| 特性 | 无点光源 | 有点光源 |
|------|---------|---------|
| **顶点着色器计算** | 很少 | 中等（法线矩阵计算） |
| **片段着色器计算** | 很少（1 次纹理采样） | 较多（多次向量运算） |
| **GPU 负载** | 低 | 中等 |
| **帧率影响** | 几乎无影响 | 轻微影响（取决于模型复杂度） |

### 实际应用场景

#### 无点光源适合：

- ✅ **UI 元素**：按钮、图标等 2D/3D UI
- ✅ **背景物体**：不需要突出显示的物体
- ✅ **性能敏感场景**：需要高帧率的场景
- ✅ **风格化渲染**：卡通风格、扁平化设计

#### 有点光源适合：

- ✅ **主要物体**：需要突出显示的重要物体
- ✅ **真实感渲染**：追求真实光照效果的场景
- ✅ **游戏场景**：需要立体感和深度感的场景
- ✅ **产品展示**：需要展示产品细节的场景

### 总结对比表

| 对比项 | 无点光源（Lesson 12.1） | 有点光源（Lesson 12.2） |
|--------|----------------------|----------------------|
| **视觉效果** | 平面、无立体感 | 立体、有明暗变化 |
| **代码复杂度** | 简单 | 复杂 |
| **计算量** | 少 | 多 |
| **性能** | 高 | 中等 |
| **真实感** | 低 | 高 |
| **适用场景** | UI、背景、风格化 | 主要物体、真实感渲染 |
| **顶点着色器** | 简单（只传递纹理坐标） | 复杂（计算法线和位置） |
| **片段着色器** | 简单（1 行代码） | 复杂（多步光照计算） |
| **需要的 uniform** | 少（只有变换矩阵和纹理） | 多（光源、材质、相机位置） |
| **纹理使用** | 只用漫反射纹理 | 用漫反射和镜面反射纹理 |

### 关键理解点

1. **为什么需要法线？**
   - 法线决定了面的朝向
   - 通过法线和光源方向的夹角，可以计算光照强度
   - 面向光源的面（法线和光源方向夹角小）更亮
   - 背向光源的面（法线和光源方向夹角大）更暗

2. **为什么需要片段位置？**
   - 用于计算从片段到光源的方向
   - 用于计算距离（距离衰减）
   - 用于计算视线方向（镜面反射）

3. **为什么需要距离衰减？**
   - 真实世界中，光线强度随距离衰减
   - 距离光源越远，光线越弱
   - 使用二次函数模拟这种衰减效果

4. **环境光的作用？**
   - 确保即使没有直接光照，物体也不会完全黑暗
   - 模拟间接光照（物体被其他物体反射的光照亮）
   - 提供基础的可见度

5. **镜面反射的作用？**
   - 模拟光滑表面的高光反射
   - 在特定角度可以看到亮点
   - 增加真实感和细节

---

## 光照模型对比

### Lesson 12.1 vs 12.2 vs 12.3

| 特性 | Lesson 12.1 | Lesson 12.2 | Lesson 12.3 |
|------|------------|-------------|-------------|
| **光照** | ❌ 无 | ✅ 点光源 | ✅ 平行光 |
| **顶点着色器** | 简单（仅传递纹理坐标） | 复杂（计算法线和片段位置） | 复杂（计算法线和片段位置） |
| **片段着色器** | 简单（仅采样纹理） | 复杂（Phong 光照 + 衰减） | 复杂（Phong 光照） |
| **光源位置** | - | ✅ 有位置 | ❌ 无位置（有方向） |
| **距离衰减** | - | ✅ 有 | ❌ 无 |
| **视觉效果** | 平面纹理 | 有立体感，局部照明 | 有立体感，全局照明 |

### 点光源 vs 平行光

| 特性 | 点光源 | 平行光 |
|------|--------|--------|
| **位置** | ✅ 有位置 | ❌ 无位置 |
| **方向** | ❌ 无方向（向所有方向） | ✅ 固定方向 |
| **距离衰减** | ✅ 有 | ❌ 无 |
| **计算复杂度** | 中等（需要计算距离） | 低（固定方向） |
| **适合场景** | 灯泡、火把 | 太阳光 |

### 光照计算对比

#### 点光源（Lesson 12.2）

```glsl
// 计算光源方向（每个片段不同）
vec3 lightDir = normalize(light.position - FragPos);

// 计算距离衰减
float distance = length(light.position - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

// 应用衰减
diffuse *= attenuation;
specular *= attenuation;
```

#### 平行光（Lesson 12.3）

```glsl
// 光源方向（所有片段相同）
vec3 lightDir = normalize(-light.direction);

// 无距离衰减
// 直接使用光照计算结果
```

---

## 使用指南

### 1. 准备模型文件

将模型文件放在以下目录：
```
engine/assets/models/backpack/
├── backpack.obj
├── diffuse.png
├── specular.png
└── ...
```

### 2. 运行程序

- **Lesson 12.1**：输入 `12`
- **Lesson 12.2**：输入 `12-2`
- **Lesson 12.3**：输入 `12-3`

### 3. 控制说明

- **WASD**：移动相机
- **空格**：向上移动
- **Shift**：向下移动
- **鼠标移动**：旋转视角
- **滚轮**：缩放视野
- **ESC**：退出程序

### 4. 修改模型路径

在对应的 `lesson12_X.cpp` 文件中修改：

```cpp
std::string modelPath = std::string(PROJECT_ROOT) + "/engine/assets/models/your_model/model.obj";
```

### 5. 调整光照参数

#### 点光源（Lesson 12.2）

```cpp
// 光源位置
m_lightPos = glm::vec3(1.2f, 1.0f, 2.0f);

// 光源颜色和强度
m_shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
m_shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
m_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

// 衰减系数（根据距离调整）
m_shader->setFloat("light.constant", 1.0f);
m_shader->setFloat("light.linear", 0.09f);
m_shader->setFloat("light.quadratic", 0.032f);
```

#### 平行光（Lesson 12.3）

```cpp
// 光源方向
m_shader->setVec3("light.direction", -0.2f, -1.0f, -0.3f);

// 光源颜色和强度
m_shader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
m_shader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
m_shader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
```

---

## 常见问题

### Q1: 模型加载失败

**A:** 检查以下几点：
1. 模型文件路径是否正确
2. 模型文件格式是否支持
3. 纹理文件是否在正确的位置
4. Assimp 库是否正确编译和链接

### Q2: 纹理不显示

**A:** 
1. 检查纹理文件路径（相对于模型文件）
2. 确保纹理文件格式支持（PNG, JPG 等）
3. 检查 `stbi_set_flip_vertically_on_load(true)` 是否设置

### Q3: 光照效果不正确

**A:**
1. 确保顶点着色器输出了 `Normal` 和 `FragPos`
2. 检查法线矩阵计算是否正确
3. 确保着色器中的 uniform 变量名称匹配
4. 检查光源参数设置是否正确

### Q4: 为什么 sampler2D 不能放在 struct 中？

**A:** 这是 GLSL 的限制。`sampler2D` 是特殊类型，不能作为 struct 的成员。必须作为独立的 uniform 变量。

### Q5: 如何加载多个模型？

**A:** 创建多个 `Model` 对象：

```cpp
Model* model1 = new Model("path/to/model1.obj");
Model* model2 = new Model("path/to/model2.obj");

// 渲染时
model1->Draw(*shader);
model2->Draw(*shader);
```

### Q6: 如何优化模型加载性能？

**A:**
1. **纹理去重**：Model 类已经实现了纹理去重优化
2. **使用索引**：使用 EBO 减少顶点数据
3. **合并网格**：如果可能，合并多个网格
4. **使用实例化渲染**：如果渲染多个相同模型

---

## 总结

### Lesson 12 的核心价值

1. **模型加载**：学会了使用 Assimp 库加载 3D 模型
2. **数据结构**：理解了 Mesh 和 Model 类的设计
3. **光照应用**：学会了在加载的模型上应用不同的光照效果
4. **代码组织**：理解了如何组织复杂的渲染代码

### 关键知识点

1. ✅ **Assimp 库**：3D 模型加载库，支持多种格式
2. ✅ **Mesh 类**：封装单个网格的数据和渲染
3. ✅ **Model 类**：管理多个网格，处理模型加载
4. ✅ **纹理命名约定**：`texture_diffuse1`, `texture_specular1` 等
5. ✅ **法线矩阵**：将法线从局部空间变换到世界空间
6. ✅ **点光源 vs 平行光**：不同光照类型的特点和应用

### 下一步学习方向

1. **高级光照**：
   - 聚光灯（Spotlight）
   - 多光源系统
   - 阴影映射（Shadow Mapping）

2. **高级渲染**：
   - 法线贴图（Normal Mapping）
   - 视差贴图（Parallax Mapping）
   - PBR（基于物理的渲染）

3. **动画**：
   - 骨骼动画（Bone Animation）
   - 关键帧动画

4. **优化**：
   - 实例化渲染（Instanced Rendering）
   - 视锥剔除（Frustum Culling）
   - LOD（细节层次）

---

## 代码结构总结

```
Lesson 12
├── lesson12_1.cpp          # 基础模型加载
├── lesson12_2.cpp          # 模型加载 + 点光源
├── lesson12_3.cpp          # 模型加载 + 平行光
├── 1.model_loading.vs      # 基础顶点着色器
├── 1.model_loading.fs      # 基础片段着色器
├── 2.model_loading_point_light.vs      # 点光源顶点着色器
├── 2.model_loading_point_light.fs      # 点光源片段着色器
├── 3.model_loading_directional_light.vs      # 平行光顶点着色器
└── 3.model_loading_directional_light.fs      # 平行光片段着色器

common/
├── mesh.h                  # Mesh 类
└── model.h                 # Model 类
```

---

这就是 Lesson 12 的完整总结！通过本课程，您已经掌握了 3D 模型加载和基础光照应用的核心技能。

