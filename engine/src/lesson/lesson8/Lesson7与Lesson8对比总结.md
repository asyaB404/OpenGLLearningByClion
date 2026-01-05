# Lesson 7 与 Lesson 8 对比总结

## 概述

Lesson 7 和 Lesson 8 都涉及基础光照，但实现方式有本质区别：
- **Lesson 7**：最简单的光照模型（颜色乘法）
- **Lesson 8**：Phong 光照模型（环境光 + 漫反射 + 镜面反射）

## 核心差异对比表

| 特性 | Lesson 7 | Lesson 8 |
|------|----------|----------|
| **光照模型** | 简单颜色乘法 | Phong 光照模型 |
| **顶点数据格式** | 位置(3) = 3个float | 位置(3) + 法线(3) = 6个float |
| **法线向量** | ❌ 无 | ✅ 有 |
| **光照计算** | `lightColor * objectColor` | `(ambient + diffuse + specular) * objectColor` |
| **光源位置** | ❌ 不需要 | ✅ 需要（用于计算方向） |
| **相机位置** | ❌ 不需要 | ✅ 需要（用于镜面反射） |
| **视觉效果** | 平面，无立体感 | 有明暗变化，有高光 |
| **方向性** | ❌ 无方向性 | ✅ 有方向性（根据光源方向） |
| **高光效果** | ❌ 无 | ✅ 有（镜面反射） |

## 详细对比

### 1. 顶点数据格式

#### Lesson 7：只有位置

```cpp
// 顶点数据：位置(3) = 3个float
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  // 位置
     0.5f, -0.5f, -0.5f,  // 位置
    // ...
};

// 顶点属性设置
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
```

**特点：**
- 每个顶点只有 3 个 float（位置）
- 步长：`3 * sizeof(float) = 12 字节`
- 只有 1 个顶点属性（位置）

#### Lesson 8：位置 + 法线

```cpp
// 顶点数据：位置(3) + 法线(3) = 6个float
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  // 位置 + 法线
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  // 位置 + 法线
    // ...
};

// 位置属性（location = 0）
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

// 法线属性（location = 1）
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
```

**特点：**
- 每个顶点有 6 个 float（位置 + 法线）
- 步长：`6 * sizeof(float) = 24 字节`
- 有 2 个顶点属性（位置和法线）

**法线向量的作用：**
- 定义面的朝向
- 用于计算光照方向
- 决定哪些面更亮，哪些面更暗

### 2. 顶点着色器

#### Lesson 7：简单变换

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;      // 只有位置

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

**特点：**
- 只处理位置
- 直接应用变换矩阵

#### Lesson 8：处理法线

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;      // 位置
layout (location = 1) in vec3 aNormal;  // 法线

out vec3 Normal;                         // 输出法线
out vec3 FragPos;                        // 输出片段位置（世界空间）

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 计算片段在世界空间中的位置
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // 将法线从局部空间变换到世界空间
    // 注意：法线矩阵 = 模型矩阵的逆矩阵的转置
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

**关键点：**
1. **FragPos**：片段在世界空间中的位置（用于计算光源方向）
2. **Normal**：法线向量（从局部空间变换到世界空间）
3. **法线矩阵**：`transpose(inverse(model))` - 确保法线在缩放后仍然正确

**为什么需要法线矩阵？**
- 如果模型有非均匀缩放，直接使用模型矩阵变换法线会导致法线不正确
- 法线矩阵确保法线始终垂直于表面

### 3. 片段着色器

#### Lesson 7：简单颜色乘法

```glsl
#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;  // 物体颜色
uniform vec3 lightColor;   // 光源颜色

void main()
{
    // 简单的颜色计算：物体颜色 × 光源颜色
    FragColor = vec4(lightColor * objectColor, 1.0);
}
```

**特点：**
- 所有面的颜色相同
- 没有方向性
- 没有明暗变化
- 没有高光

**视觉效果：**
- 立方体看起来是"平的"
- 所有面都是相同的颜色
- 没有立体感

#### Lesson 8：Phong 光照模型

```glsl
#version 330 core
out vec4 FragColor;

in vec3 Normal;           // 法线向量
in vec3 FragPos;          // 片段位置（世界空间）

uniform vec3 objectColor; // 物体颜色
uniform vec3 lightColor;  // 光源颜色
uniform vec3 lightPos;    // 光源位置（世界空间）
uniform vec3 viewPos;    // 相机位置（世界空间）

void main()
{
    // 1. 环境光（Ambient）
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // 2. 漫反射（Diffuse）
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. 镜面反射（Specular）
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // 最终颜色 = (环境光 + 漫反射 + 镜面反射) × 物体颜色
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
```

**Phong 光照模型的三个组成部分：**

#### 3.1 环境光（Ambient）

```glsl
float ambientStrength = 0.1;
vec3 ambient = ambientStrength * lightColor;
```

**作用：**
- 模拟环境中的间接光照
- 确保即使没有直接光照的地方也不会完全黑暗
- 强度通常很小（0.1）

**特点：**
- 所有面都有相同的基础亮度
- 不依赖光源方向
- 不依赖视角

#### 3.2 漫反射（Diffuse）

```glsl
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;
```

**计算步骤：**
1. 归一化法线向量
2. 计算光源方向（从片段指向光源）
3. 计算点积：`dot(norm, lightDir)`
   - 如果法线和光源方向相同（面朝向光源），点积 = 1.0（最亮）
   - 如果法线和光源方向垂直，点积 = 0.0（较暗）
   - 如果法线和光源方向相反（面背向光源），点积 < 0（使用 max 限制为 0）

**作用：**
- 根据面的朝向和光源方向计算亮度
- 朝向光源的面更亮，背向光源的面更暗
- 产生基本的明暗效果

**视觉效果：**
- 立方体的不同面有不同的亮度
- 朝向光源的面更亮
- 背向光源的面更暗

#### 3.3 镜面反射（Specular）

```glsl
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
vec3 specular = specularStrength * spec * lightColor;
```

**计算步骤：**
1. 计算视角方向（从片段指向相机）
2. 计算反射方向（光源方向关于法线的反射）
3. 计算点积：`dot(viewDir, reflectDir)`
   - 如果视角方向接近反射方向，点积接近 1.0（高光）
   - 如果视角方向远离反射方向，点积接近 0.0（无高光）
4. 使用 `pow(..., 32)` 使高光更集中（shininess = 32）

**作用：**
- 模拟光滑表面的高光效果
- 当视角接近反射方向时产生亮点
- 增加真实感

**视觉效果：**
- 在光滑表面上产生亮点（高光）
- 高光位置随视角变化
- 使物体看起来更有光泽

### 4. Uniform 变量

#### Lesson 7：只需要颜色

```cpp
lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
```

**Uniform 变量：**
- `objectColor`：物体颜色
- `lightColor`：光源颜色

#### Lesson 8：需要位置信息

```cpp
lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
lightingShader.setVec3("lightPos", lightPos);        // 新增
lightingShader.setVec3("viewPos", camera.Position);  // 新增
```

**Uniform 变量：**
- `objectColor`：物体颜色
- `lightColor`：光源颜色
- `lightPos`：光源位置（用于计算光源方向）
- `viewPos`：相机位置（用于计算视角方向和镜面反射）

### 5. 视觉效果对比

#### Lesson 7 的视觉效果

```
立方体：
- 所有面都是相同的颜色
- 没有明暗变化
- 没有高光
- 看起来是"平的"
- 没有立体感
```

**示例：**
- 橙色立方体，所有面都是相同的橙色
- 无论光源在哪里，颜色都一样
- 无论从哪个角度看，颜色都一样

#### Lesson 8 的视觉效果

```
立方体：
- 不同面有不同的亮度
- 朝向光源的面更亮
- 背向光源的面更暗
- 有高光效果（镜面反射）
- 看起来有立体感
- 更真实
```

**示例：**
- 橙色立方体，但不同面亮度不同
- 朝向光源的面更亮（接近白色）
- 背向光源的面更暗（接近黑色）
- 在光滑面上有亮点（高光）
- 移动相机时，高光位置会变化

## 代码结构对比

### Lesson 7 的代码结构

```cpp
// 顶点数据：只有位置
float vertices[] = { /* 位置 */ };

// 顶点属性：只有位置
glVertexAttribPointer(0, 3, ...);  // 位置

// 着色器：简单颜色乘法
FragColor = vec4(lightColor * objectColor, 1.0);
```

### Lesson 8 的代码结构

```cpp
// 顶点数据：位置 + 法线
float vertices[] = { /* 位置 + 法线 */ };

// 顶点属性：位置和法线
glVertexAttribPointer(0, 3, ...);  // 位置
glVertexAttribPointer(1, 3, ...);  // 法线

// 着色器：Phong 光照模型
vec3 ambient = ...;   // 环境光
vec3 diffuse = ...;   // 漫反射
vec3 specular = ...;  // 镜面反射
FragColor = vec4((ambient + diffuse + specular) * objectColor, 1.0);
```

## 关键概念理解

### 1. 法线向量（Normal Vector）

**定义：**
- 垂直于表面的向量
- 指向表面的"外"方向
- 用于计算光照

**作用：**
- 定义面的朝向
- 计算光源方向与面的夹角
- 决定面的亮度

**示例：**
```
立方体的前面（+Z方向）：
- 法线 = (0, 0, 1)  // 指向 +Z 方向

立方体的右面（+X方向）：
- 法线 = (1, 0, 0)  // 指向 +X 方向

立方体的上面（+Y方向）：
- 法线 = (0, 1, 0)  // 指向 +Y 方向
```

### 2. 点积（Dot Product）

**公式：**
```
dot(a, b) = a.x * b.x + a.y * b.y + a.z * b.z
```

**几何意义：**
- 如果两个向量方向相同，点积 = 1.0（最大值）
- 如果两个向量垂直，点积 = 0.0
- 如果两个向量方向相反，点积 = -1.0（最小值）

**在光照中的应用：**
```glsl
float diff = max(dot(norm, lightDir), 0.0);
```
- `norm` 和 `lightDir` 方向相同时，`diff = 1.0`（最亮）
- `norm` 和 `lightDir` 垂直时，`diff = 0.0`（较暗）
- `norm` 和 `lightDir` 相反时，`diff = 0.0`（最暗，因为 max(..., 0.0)）

### 3. 反射向量（Reflection Vector）

**计算：**
```glsl
vec3 reflectDir = reflect(-lightDir, norm);
```

**作用：**
- 计算光线在表面上的反射方向
- 用于镜面反射计算
- 当视角方向接近反射方向时，产生高光

### 4. 幂函数（Power Function）

**公式：**
```glsl
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
```

**作用：**
- `pow(..., 32)` 使高光更集中
- 指数越大，高光越小越亮
- 指数越小，高光越大越暗

**示例：**
- `pow(0.9, 32) ≈ 0.03`（高光很小）
- `pow(0.9, 8) ≈ 0.43`（高光较大）

## 学习要点总结

### Lesson 7 的学习要点

1. ✅ **基础光照概念**：物体颜色 × 光源颜色
2. ✅ **简单颜色计算**：分量乘法
3. ✅ **多个 VAO 的使用**：被光照物体和光源本身
4. ✅ **多个着色器的使用**：不同的着色器用于不同的物体

### Lesson 8 的学习要点

1. ✅ **法线向量**：定义面的朝向
2. ✅ **Phong 光照模型**：环境光 + 漫反射 + 镜面反射
3. ✅ **方向性光照**：根据光源方向计算亮度
4. ✅ **高光效果**：镜面反射产生的高光
5. ✅ **法线矩阵**：正确变换法线向量
6. ✅ **世界空间计算**：在片段着色器中使用世界空间坐标

## 进阶方向

### 下一步可以学习的内容

1. **Blinn-Phong 光照模型**
   - 改进的镜面反射计算
   - 使用半角向量（Halfway Vector）
   - 更高效的计算方式

2. **材质系统（Materials）**
   - 不同材质有不同的环境光、漫反射、镜面反射系数
   - 例如：金属、塑料、木头等

3. **光照贴图（Light Maps）**
   - 使用纹理控制光照
   - 漫反射贴图、镜面反射贴图

4. **多光源系统**
   - 多个点光源
   - 方向光（Directional Light）
   - 聚光灯（Spotlight）

5. **基于物理的渲染（PBR）**
   - 更真实的光照模型
   - 能量守恒
   - 更复杂的计算

## 实验建议

### 实验 1：调整环境光强度

```glsl
float ambientStrength = 0.1;  // 改为 0.0, 0.5, 1.0
```

**观察：**
- 环境光强度为 0 时，背光面完全黑暗
- 环境光强度为 1.0 时，所有面都很亮

### 实验 2：调整镜面反射强度

```glsl
float specularStrength = 0.5;  // 改为 0.0, 1.0, 2.0
```

**观察：**
- 强度为 0 时，没有高光
- 强度越大，高光越明显

### 实验 3：调整高光指数（Shininess）

```glsl
float spec = pow(..., 32);  // 改为 8, 64, 128
```

**观察：**
- 指数越小，高光越大越柔和
- 指数越大，高光越小越锐利

### 实验 4：改变光源位置

```cpp
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);  // 改变位置
```

**观察：**
- 不同面的亮度会变化
- 高光位置会变化

## 常见问题

### Q1: 为什么需要法线矩阵？

**A:** 如果模型有非均匀缩放（例如 X 轴缩放 2 倍，Y 轴缩放 1 倍），直接使用模型矩阵变换法线会导致法线不再垂直于表面。法线矩阵（模型矩阵的逆矩阵的转置）确保法线始终正确。

### Q2: 为什么使用 `max(dot(...), 0.0)`？

**A:** 点积可能为负值（当面背向光源时），但我们不希望负的光照值。`max(..., 0.0)` 确保光照值始终为非负数。

### Q3: 为什么镜面反射需要相机位置？

**A:** 镜面反射的效果取决于视角。当视角接近反射方向时，高光更明显。因此需要相机位置来计算视角方向。

### Q4: 环境光、漫反射、镜面反射的比例是多少？

**A:** 这取决于材质：
- **金属**：镜面反射强，漫反射弱
- **塑料**：漫反射和镜面反射都较强
- **木头**：漫反射强，镜面反射弱
- **布料**：主要是漫反射，几乎没有镜面反射

## 总结

### Lesson 7 → Lesson 8 的进步

1. **从简单到复杂**：从颜色乘法到 Phong 光照模型
2. **从平面到立体**：从无方向性到有方向性的光照
3. **从单调到丰富**：从单一颜色到有明暗变化和高光
4. **从基础到进阶**：为学习更高级的光照模型打下基础

### 关键收获

- ✅ 理解了法线向量的作用和重要性
- ✅ 掌握了 Phong 光照模型的三个组成部分
- ✅ 学会了在着色器中进行向量计算
- ✅ 理解了方向性光照的计算原理
- ✅ 掌握了高光效果（镜面反射）的实现

这些知识是学习更高级光照技术的基础！

