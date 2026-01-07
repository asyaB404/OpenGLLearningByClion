# Lesson 11: 光源类型总结

## 📚 目录
- [概述](#概述)
- [三种光源类型对比](#三种光源类型对比)
- [方向光（Directional Light）](#方向光directional-light)
- [点光源（Point Light）](#点光源point-light)
- [聚光灯（Spotlight）](#聚光灯spotlight)
- [如何选择光源类型](#如何选择光源类型)
- [实现细节对比](#实现细节对比)
- [思考与总结](#思考与总结)

---

## 概述

Lesson 11 介绍了三种常见的光源类型：
1. **方向光（Directional Light）** - Lesson 11.1
2. **点光源（Point Light）** - Lesson 11.2
3. **聚光灯（Spotlight）** - Lesson 11.3

每种光源类型都有不同的特性和适用场景，理解它们的区别对于创建真实的光照效果至关重要。

---

## 三种光源类型对比

### 快速对比表

| 特性 | 方向光 | 点光源 | 聚光灯 |
|------|--------|--------|--------|
| **位置** | ❌ 无（无限远） | ✅ 有位置 | ✅ 有位置 |
| **方向** | ✅ 固定方向 | ❌ 无方向 | ✅ 有方向 |
| **角度限制** | ❌ 无 | ❌ 无 | ✅ 有（内角/外角） |
| **距离衰减** | ❌ 无 | ✅ 有 | ✅ 有 |
| **光源可视化** | ❌ 不渲染 | ✅ 渲染光源立方体 | ❌ 不渲染（跟随相机） |
| **适合场景** | 太阳光 | 灯泡、火把 | 手电筒、聚光灯 |
| **计算复杂度** | 低 | 中 | 高 |

### 详细对比

#### 1. 光源属性

**方向光（Directional Light）：**
```glsl
struct Light {
    vec3 direction;    // 光源方向（固定）
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
```

**点光源（Point Light）：**
```glsl
struct Light {
    vec3 position;     // 光源位置
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;    // 衰减系数
    float linear;
    float quadratic;
};
```

**聚光灯（Spotlight）：**
```glsl
struct Light {
    vec3 position;     // 光源位置
    vec3 direction;    // 光源方向
    float cutOff;      // 内角余弦值
    float outerCutOff; // 外角余弦值
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;    // 衰减系数
    float linear;
    float quadratic;
};
```

#### 2. 光源方向计算

**方向光：**
```glsl
// 所有片段使用相同的方向
vec3 lightDir = normalize(-light.direction);
```

**点光源：**
```glsl
// 每个片段计算不同的方向（从片段指向光源）
vec3 lightDir = normalize(light.position - FragPos);
```

**聚光灯：**
```glsl
// 每个片段计算不同的方向（从片段指向光源）
vec3 lightDir = normalize(light.position - FragPos);
// 然后检查是否在聚光灯角度内
```

---

## 方向光（Directional Light）

### 特点

1. **没有位置**：来自无限远
2. **固定方向**：所有片段接收到的光线方向相同
3. **无距离衰减**：距离不影响光照强度
4. **适合模拟太阳光**：太阳距离地球很远，可以近似为方向光

### 实现

#### 着色器结构体

```glsl
struct Light {
    vec3 direction;    // 光源方向（世界空间）
    vec3 ambient;       // 环境光颜色
    vec3 diffuse;       // 漫反射颜色
    vec3 specular;      // 镜面反射颜色
};
```

#### 光源方向计算

```glsl
// 方向光：光源方向是固定的，不需要计算
vec3 lightDir = normalize(-light.direction);  // 注意：取反
```

**为什么取反？**
- `light.direction` 指向光源
- 但我们需要从片段指向光源的方向
- 所以需要取反：`-light.direction`

#### C++ 代码设置

```cpp
// 设置光源方向
lightingShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
// 注意：这里不需要取反，因为已经在着色器中取反了
```

### 使用场景

- **太阳光**：模拟白天场景
- **远距离光源**：距离很远，可以近似为方向光
- **全局光照**：需要均匀照亮整个场景

### 优点

- ✅ 计算简单（不需要计算每个片段的方向）
- ✅ 性能好（无距离衰减计算）
- ✅ 适合大场景

### 缺点

- ❌ 无法表现距离感
- ❌ 所有物体接收相同的光照方向
- ❌ 不够真实（真实世界中很少有完全平行的光）

---

## 点光源（Point Light）

### 特点

1. **有位置**：光源在世界空间中的具体位置
2. **无方向限制**：向所有方向发射光线
3. **距离衰减**：光线强度随距离衰减
4. **适合模拟灯泡、火把等**：真实世界中的点光源

### 实现

#### 着色器结构体

```glsl
struct Light {
    vec3 position;      // 光源位置（世界空间）
    vec3 ambient;       // 环境光颜色
    vec3 diffuse;       // 漫反射颜色
    vec3 specular;      // 镜面反射颜色
    
    // 衰减系数
    float constant;     // 常数项（通常为 1.0）
    float linear;       // 线性项
    float quadratic;   // 二次项
};
```

#### 光源方向计算

```glsl
// 点光源：计算从片段到光源的方向
vec3 lightDir = normalize(light.position - FragPos);
```

#### 距离衰减计算

```glsl
// 计算距离
float distance = length(light.position - FragPos);

// 计算衰减系数
float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

// 应用衰减（环境光不受衰减影响）
ambient *= attenuation;
diffuse *= attenuation;
specular *= attenuation;
```

**衰减公式：**
```
attenuation = 1.0 / (constant + linear * distance + quadratic * distance²)
```

**衰减系数参考值：**

| 距离 | constant | linear | quadratic |
|------|----------|--------|-----------|
| 7 | 1.0 | 0.7 | 1.8 |
| 13 | 1.0 | 0.35 | 0.44 |
| 20 | 1.0 | 0.22 | 0.20 |
| 32 | 1.0 | 0.14 | 0.07 |
| 50 | 1.0 | 0.09 | 0.032 |
| 65 | 1.0 | 0.07 | 0.017 |
| 100 | 1.0 | 0.045 | 0.0075 |
| 160 | 1.0 | 0.027 | 0.0028 |
| 200 | 1.0 | 0.022 | 0.0019 |
| 325 | 1.0 | 0.014 | 0.0007 |
| 600 | 1.0 | 0.007 | 0.0002 |
| 3250 | 1.0 | 0.0014 | 0.000007 |

#### C++ 代码设置

```cpp
// 设置光源位置
lightingShader.setVec3("light.position", lightPos);

// 设置衰减系数
lightingShader.setFloat("light.constant", 1.0f);
lightingShader.setFloat("light.linear", 0.09f);
lightingShader.setFloat("light.quadratic", 0.032f);
```

### 使用场景

- **灯泡**：室内照明
- **火把**：游戏中的光源
- **蜡烛**：小范围照明
- **任何有位置的光源**

### 优点

- ✅ 真实感强（有距离衰减）
- ✅ 可以表现距离感
- ✅ 适合局部照明

### 缺点

- ❌ 计算稍复杂（需要计算距离和衰减）
- ❌ 性能开销稍大（每个片段都要计算）

---

## 聚光灯（Spotlight）

### 特点

1. **有位置**：光源在世界空间中的具体位置
2. **有方向**：光线只向特定方向发射
3. **角度限制**：有内角（cutOff）和外角（outerCutOff）
4. **距离衰减**：光线强度随距离衰减
5. **适合模拟手电筒、聚光灯等**：真实世界中的聚光灯

### 实现

#### 着色器结构体

```glsl
struct Light {
    vec3 position;      // 光源位置（世界空间）
    vec3 direction;     // 光源方向（世界空间）
    float cutOff;       // 内角余弦值（内圆锥）
    float outerCutOff;  // 外角余弦值（外圆锥）
    
    vec3 ambient;       // 环境光颜色
    vec3 diffuse;       // 漫反射颜色
    vec3 specular;      // 镜面反射颜色
    
    // 衰减系数
    float constant;     // 常数项
    float linear;      // 线性项
    float quadratic;   // 二次项
};
```

#### 聚光灯强度计算

```glsl
// 计算片段到光源的方向
vec3 lightDir = normalize(light.position - FragPos);

// 计算片段方向与光源方向的夹角余弦值
float theta = dot(lightDir, normalize(-light.direction));

// 计算内角和外角的差值
float epsilon = light.cutOff - light.outerCutOff;

// 计算聚光灯强度（0.0 到 1.0）
float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

// 应用聚光灯强度
diffuse *= intensity;
specular *= intensity;
```

**强度计算详解：**

1. **theta**：片段方向与光源方向的夹角余弦值
   - `theta = dot(lightDir, normalize(-light.direction))`
   - 范围：`[-1.0, 1.0]`

2. **epsilon**：内角和外角的差值
   - `epsilon = cutOff - outerCutOff`
   - 例如：`cos(12.5°) - cos(17.5°) ≈ 0.978 - 0.954 = 0.024`

3. **intensity**：聚光灯强度
   - `intensity = (theta - outerCutOff) / epsilon`
   - 范围：`[0.0, 1.0]`
   - 在内角内：`intensity = 1.0`（完全照亮）
   - 在外角外：`intensity = 0.0`（无光照）
   - 在内角和外角之间：`intensity` 从 1.0 渐变到 0.0

**为什么使用余弦值？**

- 点积计算返回余弦值：`dot(a, b) = |a| * |b| * cos(θ)`
- 归一化后：`dot(normalize(a), normalize(b)) = cos(θ)`
- 直接使用余弦值，避免计算角度，更高效

#### C++ 代码设置

```cpp
// 聚光灯属性（手电筒效果：光源位置和方向跟随相机）
lightingShader.setVec3("light.position", camera.Position);   // 光源位置 = 相机位置
lightingShader.setVec3("light.direction", camera.Front);      // 光源方向 = 相机前方向量

// 聚光灯角度（使用余弦值）
lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));      // 内角 12.5 度
lightingShader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f))); // 外角 17.5 度
```

### 使用场景

- **手电筒**：第一人称游戏中的手电筒
- **聚光灯**：舞台灯光、探照灯
- **车灯**：汽车前灯
- **任何有方向限制的光源**

### 优点

- ✅ 真实感最强（有位置、方向、角度限制、距离衰减）
- ✅ 可以创建戏剧性的光照效果
- ✅ 适合局部照明

### 缺点

- ❌ 计算最复杂（需要计算角度和强度）
- ❌ 性能开销最大

---

## 如何选择光源类型

### 选择指南

#### 使用方向光，当：

- ✅ 需要模拟太阳光
- ✅ 需要均匀照亮整个场景
- ✅ 性能要求高
- ✅ 不需要距离感

**示例场景：**
- 室外场景（白天）
- 大范围照明
- 性能敏感的场景

#### 使用点光源，当：

- ✅ 需要模拟灯泡、火把等
- ✅ 需要距离衰减效果
- ✅ 光源位置固定
- ✅ 不需要方向限制

**示例场景：**
- 室内照明
- 游戏中的火把、蜡烛
- 需要表现距离感的场景

#### 使用聚光灯，当：

- ✅ 需要模拟手电筒、聚光灯
- ✅ 需要方向限制
- ✅ 需要创建戏剧性的光照效果
- ✅ 光源位置和方向可以变化

**示例场景：**
- 第一人称游戏中的手电筒
- 舞台灯光
- 需要局部照明的场景

### 组合使用

在实际应用中，通常会**组合使用多种光源类型**：

```glsl
// 方向光（太阳光）
DirectionalLight sunLight;

// 多个点光源（室内灯光）
PointLight roomLights[4];

// 聚光灯（手电筒）
Spotlight flashlight;
```

**示例场景：**
- 室外场景：方向光（太阳）+ 点光源（路灯）
- 室内场景：点光源（室内灯）+ 聚光灯（手电筒）
- 游戏场景：方向光（环境光）+ 点光源（火把）+ 聚光灯（手电筒）

---

## 实现细节对比

### 1. 光源方向计算

#### 方向光

```glsl
// 所有片段使用相同的方向
vec3 lightDir = normalize(-light.direction);
```

**特点：**
- 计算一次，所有片段共享
- 性能最好

#### 点光源

```glsl
// 每个片段计算不同的方向
vec3 lightDir = normalize(light.position - FragPos);
```

**特点：**
- 每个片段都要计算
- 需要计算向量差和归一化

#### 聚光灯

```glsl
// 每个片段计算不同的方向（与点光源相同）
vec3 lightDir = normalize(light.position - FragPos);

// 然后检查角度
float theta = dot(lightDir, normalize(-light.direction));
```

**特点：**
- 与点光源相同，但还要检查角度
- 计算最复杂

### 2. 距离衰减

#### 方向光

```glsl
// 无距离衰减
// 不需要计算
```

#### 点光源和聚光灯

```glsl
// 计算距离
float distance = length(light.position - FragPos);

// 计算衰减系数
float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

// 应用衰减
diffuse *= attenuation;
specular *= attenuation;
```

**衰减公式详解：**

```
attenuation = 1.0 / (constant + linear * distance + quadratic * distance²)
```

**各系数的作用：**
- **constant**：常数项，通常为 1.0，确保近距离时衰减不会太大
- **linear**：线性项，控制近距离衰减
- **quadratic**：二次项，控制远距离衰减

**衰减曲线：**
- 近距离：衰减主要由 constant 控制
- 中距离：衰减主要由 linear 控制
- 远距离：衰减主要由 quadratic 控制

### 3. 角度限制（仅聚光灯）

```glsl
// 计算片段方向与光源方向的夹角
float theta = dot(lightDir, normalize(-light.direction));

// 计算强度
float epsilon = light.cutOff - light.outerCutOff;
float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

// 应用强度
diffuse *= intensity;
specular *= intensity;
```

**角度区域：**

```
内角内（theta >= cutOff）：
  intensity = 1.0（完全照亮）

内角和外角之间（outerCutOff < theta < cutOff）：
  intensity = (theta - outerCutOff) / epsilon（渐变）

外角外（theta <= outerCutOff）：
  intensity = 0.0（无光照）
```

### 4. 性能对比

| 光源类型 | 每片段计算量 | 性能开销 |
|---------|------------|---------|
| **方向光** | 1 次向量归一化 | 最低 |
| **点光源** | 1 次向量差 + 1 次归一化 + 1 次距离计算 + 1 次衰减计算 | 中等 |
| **聚光灯** | 点光源的计算 + 1 次点积 + 1 次强度计算 | 最高 |

**优化建议：**
- 如果不需要距离衰减，使用方向光
- 如果不需要角度限制，使用点光源
- 只在必要时使用聚光灯

---

## 详细代码对比

### 1. 片段着色器对比

#### 方向光（Lesson 11.1）

```glsl
// 光源方向（固定）
vec3 lightDir = normalize(-light.direction);

// 漫反射
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoord)));

// 镜面反射
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));

// 无衰减
vec3 result = ambient + diffuse + specular;
```

#### 点光源（Lesson 11.2）

```glsl
// 光源方向（每个片段不同）
vec3 lightDir = normalize(light.position - FragPos);

// 漫反射
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoord)));

// 镜面反射
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));

// 距离衰减
float distance = length(light.position - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
diffuse *= attenuation;
specular *= attenuation;

vec3 result = ambient + diffuse + specular;
```

#### 聚光灯（Lesson 11.3）

```glsl
// 光源方向（每个片段不同）
vec3 lightDir = normalize(light.position - FragPos);

// 漫反射
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoord)));

// 镜面反射
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));

// 聚光灯强度
float theta = dot(lightDir, normalize(-light.direction));
float epsilon = light.cutOff - light.outerCutOff;
float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
diffuse *= intensity;
specular *= intensity;

// 距离衰减
float distance = length(light.position - FragPos);
float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
diffuse *= attenuation;
specular *= attenuation;

vec3 result = ambient + diffuse + specular;
```

### 2. C++ 代码设置对比

#### 方向光

```cpp
// 设置光源方向
lightingShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);

// 光源属性
lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
```

#### 点光源

```cpp
// 设置光源位置
lightingShader.setVec3("light.position", lightPos);

// 光源属性
lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

// 衰减系数
lightingShader.setFloat("light.constant", 1.0f);
lightingShader.setFloat("light.linear", 0.09f);
lightingShader.setFloat("light.quadratic", 0.032f);
```

#### 聚光灯

```cpp
// 设置光源位置和方向（跟随相机）
lightingShader.setVec3("light.position", camera.Position);
lightingShader.setVec3("light.direction", camera.Front);

// 聚光灯角度
lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
lightingShader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));

// 光源属性
lightingShader.setVec3("light.ambient", 0.1f, 0.1f, 0.1f);
lightingShader.setVec3("light.diffuse", 0.8f, 0.8f, 0.8f);
lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

// 衰减系数
lightingShader.setFloat("light.constant", 1.0f);
lightingShader.setFloat("light.linear", 0.09f);
lightingShader.setFloat("light.quadratic", 0.032f);
```

---

## 思考与总结

### 1. 三种光源的关系

#### 方向光 → 点光源

**方向光可以看作是点光源的特殊情况：**
- 点光源在无限远时，所有片段到光源的方向相同
- 点光源在无限远时，距离衰减可以忽略

**数学关系：**
```
当 light.position → ∞ 时：
  lightDir = normalize(light.position - FragPos) ≈ normalize(light.position) = 固定方向
  distance → ∞，但衰减可以忽略（如果不需要衰减）
```

#### 点光源 → 聚光灯

**聚光灯是点光源的扩展：**
- 点光源 + 方向限制 = 聚光灯
- 聚光灯在角度限制为 180 度时 = 点光源

**数学关系：**
```
当 cutOff = 1.0, outerCutOff = 1.0 时：
  intensity = 1.0（所有角度都照亮）
  聚光灯 = 点光源
```

### 2. 为什么需要不同的光源类型？

#### 真实世界的需求

**真实世界中的光源：**
- **太阳**：距离很远，可以近似为方向光
- **灯泡**：有位置，向所有方向发光，有距离衰减
- **手电筒**：有位置，有方向，有角度限制，有距离衰减

#### 性能考虑

**不同光源类型的性能：**
- 方向光：性能最好（计算最简单）
- 点光源：性能中等（需要计算距离和衰减）
- 聚光灯：性能最差（需要计算角度和强度）

**优化策略：**
- 大范围照明：使用方向光
- 局部照明：使用点光源
- 特殊效果：使用聚光灯

### 3. 实际应用中的组合

#### 典型场景组合

**室外场景（白天）：**
```glsl
DirectionalLight sunLight;  // 太阳光（主光源）
PointLight streetLights[10]; // 路灯（辅助光源）
```

**室内场景：**
```glsl
DirectionalLight windowLight; // 窗户进来的光（环境光）
PointLight roomLights[4];     // 室内灯光（主光源）
```

**游戏场景：**
```glsl
DirectionalLight ambientLight; // 环境光（全局）
PointLight torches[5];          // 火把（局部照明）
Spotlight playerFlashlight;     // 玩家手电筒（跟随玩家）
```

### 4. 光源强度设置

#### 方向光强度

```cpp
// 方式1：直接设置各分量
lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

// 方式2：使用强度系数（推荐）
float intensity = 1.0f;
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
glm::vec3 ambient = lightColor * intensity * 0.2f;
glm::vec3 diffuse = lightColor * intensity * 0.5f;
glm::vec3 specular = lightColor * intensity * 1.0f;
```

#### 点光源和聚光灯强度

**除了设置各分量，还要考虑距离衰减：**
```glsl
// 最终强度 = 光源强度 × 距离衰减 × 角度强度（仅聚光灯）
finalIntensity = lightIntensity * attenuation * intensity;
```

### 5. 常见问题

#### Q1: 为什么方向光不需要位置？

**A:** 方向光来自无限远，所有片段接收到的光线方向相同。位置信息没有意义，因为无论片段在哪里，光线方向都一样。

#### Q2: 为什么聚光灯使用余弦值而不是角度？

**A:** 
- 点积计算返回余弦值：`dot(a, b) = cos(θ)`
- 直接使用余弦值，避免角度计算，更高效
- 在着色器中，避免使用 `acos()` 等函数

#### Q3: 如何调整聚光灯的角度？

**A:** 
```cpp
// 内角：完全照亮区域的角度
float innerAngle = 12.5f;  // 度
lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(innerAngle)));

// 外角：边缘衰减区域的角度
float outerAngle = 17.5f;  // 度
lightingShader.setFloat("light.outerCutOff", glm::cos(glm::radians(outerAngle)));
```

**角度选择：**
- 内角和外角差值越大，边缘越柔和
- 内角和外角差值越小，边缘越锐利
- 典型值：内角 10-15 度，外角 15-20 度

#### Q4: 如何选择合适的衰减系数？

**A:** 根据光源的预期照射距离选择：

| 距离 | constant | linear | quadratic |
|------|----------|--------|-----------|
| 近距离（7） | 1.0 | 0.7 | 1.8 |
| 中距离（50） | 1.0 | 0.09 | 0.032 |
| 远距离（325） | 1.0 | 0.014 | 0.0007 |

**选择原则：**
- 近距离光源：linear 和 quadratic 较大
- 远距离光源：linear 和 quadratic 较小

---

## 总结

### Lesson 11 的核心价值

1. **理解三种光源类型**：方向光、点光源、聚光灯
2. **掌握光源特性**：位置、方向、角度、衰减
3. **学会选择光源类型**：根据场景需求选择合适的光源
4. **实现真实光照**：使用合适的光源创建真实的光照效果

### 三种光源的演进

```
方向光（最简单）
  ↓ 添加位置和距离衰减
点光源（中等复杂）
  ↓ 添加方向和角度限制
聚光灯（最复杂）
```

### 关键收获

1. ✅ **理解了光源类型**：为什么需要不同的光源类型
2. ✅ **掌握了实现方式**：如何在着色器中实现各种光源
3. ✅ **学会了性能优化**：根据需求选择合适的光源类型
4. ✅ **理解了衰减和角度**：距离衰减和角度限制的计算方式

### 下一步学习方向

1. **多光源系统**
   - 同时使用多种光源类型
   - 光源管理

2. **阴影系统**
   - 阴影映射（Shadow Mapping）
   - 软阴影

3. **高级光照技术**
   - 延迟渲染（Deferred Rendering）
   - 基于物理的渲染（PBR）

---

## 实验建议

### 实验 1：改变方向光方向

```cpp
// 尝试不同的方向
lightingShader.setVec3("light.direction", 0.0f, -1.0f, 0.0f);  // 从上往下
lightingShader.setVec3("light.direction", 1.0f, -1.0f, 0.0f);  // 从右上角
```

### 实验 2：改变点光源位置

```cpp
// 移动光源位置
lightPos = glm::vec3(sin(time), 1.0f, cos(time));  // 圆形移动
```

### 实验 3：调整聚光灯角度

```cpp
// 窄角度（更集中）
lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(5.0f)));
lightingShader.setFloat("light.outerCutOff", glm::cos(glm::radians(10.0f)));

// 宽角度（更分散）
lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(25.0f)));
lightingShader.setFloat("light.outerCutOff", glm::cos(glm::radians(30.0f)));
```

### 实验 4：调整衰减系数

```cpp
// 强衰减（近距离）
lightingShader.setFloat("light.linear", 0.7f);
lightingShader.setFloat("light.quadratic", 1.8f);

// 弱衰减（远距离）
lightingShader.setFloat("light.linear", 0.014f);
lightingShader.setFloat("light.quadratic", 0.0007f);
```

---

## 最终总结

Lesson 11 展示了三种常见的光源类型：

1. **方向光**：简单、高效，适合大范围照明
2. **点光源**：真实、灵活，适合局部照明
3. **聚光灯**：复杂、真实，适合特殊效果

**核心思想：**
- 根据场景需求选择合适的光源类型
- 理解每种光源的特性和适用场景
- 在实际应用中，通常会组合使用多种光源类型

这就是光源类型的精髓！

