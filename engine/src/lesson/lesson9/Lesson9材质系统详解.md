# Lesson 9: 材质系统（Materials）详解

## 📚 目录
- [概述](#概述)
- [核心概念](#核心概念)
- [与 Lesson 8 的对比](#与-lesson-8-的对比)
- [为什么需要材质系统](#为什么需要材质系统)
- [如何实现材质系统](#如何实现材质系统)
- [详细代码解析](#详细代码解析)
- [思考与总结](#思考与总结)

---

## 概述

Lesson 9 引入了**材质系统（Material System）**，这是光照系统的重要扩展。与 Lesson 8 相比，Lesson 9 将物体颜色和光源颜色分离，使用**结构体（Struct）**来组织材质和光源的属性，使得光照计算更加灵活和真实。

### 核心改进

1. **分离物体颜色和光源颜色**：不再使用简单的 `objectColor × lightColor`
2. **使用结构体组织数据**：Material 和 Light 结构体
3. **独立的反射系数**：环境光、漫反射、镜面反射各自有独立的系数
4. **动态光源颜色**：光源颜色随时间变化，展示材质系统的灵活性

---

## 核心概念

### 1. 材质（Material）

材质定义了物体**如何反射光线**，包括：

- **ambient（环境光反射系数）**：物体对环境光的反射能力
- **diffuse（漫反射系数）**：物体对漫反射光的反射能力
- **specular（镜面反射系数）**：物体对镜面反射光的反射能力
- **shininess（高光指数）**：控制高光的大小和锐度

### 2. 光源（Light）

光源定义了**光源的属性**，包括：

- **position（位置）**：光源在世界空间中的位置
- **ambient（环境光颜色）**：光源发出的环境光颜色
- **diffuse（漫反射颜色）**：光源发出的漫反射光颜色
- **specular（镜面反射颜色）**：光源发出的镜面反射光颜色

### 3. 光照计算

最终颜色 = 环境光 + 漫反射 + 镜面反射

其中：
- **环境光** = `light.ambient × material.ambient`
- **漫反射** = `light.diffuse × (diff × material.diffuse)`
- **镜面反射** = `light.specular × (spec × material.specular)`

---

## 与 Lesson 8 的对比

### 对比表格

| 特性 | Lesson 8 | Lesson 9 |
|------|----------|----------|
| **数据组织** | 独立的 uniform 变量 | Material 和 Light 结构体 |
| **物体颜色** | `objectColor`（单一颜色） | `material.ambient/diffuse/specular`（分别设置） |
| **光源颜色** | `lightColor`（单一颜色） | `light.ambient/diffuse/specular`（分别设置） |
| **高光指数** | 硬编码（32） | `material.shininess`（可配置） |
| **灵活性** | 低（所有面使用相同颜色） | 高（可以为不同材质设置不同属性） |
| **真实性** | 中等 | 高（更接近真实材质） |
| **代码组织** | 简单 | 更结构化 |

### 详细对比

#### 1. 着色器 Uniform 变量

**Lesson 8：**
```glsl
uniform vec3 objectColor;  // 物体颜色
uniform vec3 lightColor;   // 光源颜色
uniform vec3 lightPos;     // 光源位置
uniform vec3 viewPos;      // 相机位置
```

**Lesson 9：**
```glsl
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;  // 材质
uniform Light light;         // 光源
uniform vec3 viewPos;        // 相机位置
```

#### 2. 光照计算

**Lesson 8：**
```glsl
// 环境光
float ambientStrength = 0.1;
vec3 ambient = ambientStrength * lightColor;

// 漫反射
vec3 diffuse = diff * lightColor;

// 镜面反射
float specularStrength = 0.5;
vec3 specular = specularStrength * spec * lightColor;

// 最终颜色
vec3 result = (ambient + diffuse + specular) * objectColor;
```

**Lesson 9：**
```glsl
// 环境光
vec3 ambient = light.ambient * material.ambient;

// 漫反射
vec3 diffuse = light.diffuse * (diff * material.diffuse);

// 镜面反射
vec3 specular = light.specular * (spec * material.specular);

// 最终颜色
vec3 result = ambient + diffuse + specular;
```

**关键区别：**
- Lesson 8：先计算光照强度，最后乘以物体颜色
- Lesson 9：每个光照分量都分别乘以对应的材质系数和光源颜色

#### 3. C++ 代码设置

**Lesson 8：**
```cpp
lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
lightingShader.setVec3("lightPos", lightPos);
```

**Lesson 9：**
```cpp
// 光源属性
lightingShader.setVec3("light.position", lightPos);
lightingShader.setVec3("light.ambient", ambientColor);
lightingShader.setVec3("light.diffuse", diffuseColor);
lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

// 材质属性
lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
lightingShader.setFloat("material.shininess", 32.0f);
```

---

## 为什么需要材质系统？

### 1. 真实世界的需求

在真实世界中，不同材质对光线的反射方式不同：

- **金属**：镜面反射强，漫反射弱
- **塑料**：漫反射和镜面反射都较强
- **木头**：漫反射强，镜面反射弱
- **布料**：主要是漫反射，几乎没有镜面反射

**Lesson 8 的问题：**
- 所有材质都使用相同的反射方式
- 无法区分不同材质

**Lesson 9 的解决方案：**
- 每种材质可以设置不同的反射系数
- 可以模拟不同材质的效果

### 2. 灵活性需求

**Lesson 8 的限制：**
```glsl
// 所有光照分量使用相同的光源颜色
vec3 ambient = ambientStrength * lightColor;
vec3 diffuse = diff * lightColor;
vec3 specular = specularStrength * spec * lightColor;
```

**问题：**
- 无法为环境光、漫反射、镜面反射设置不同的颜色
- 例如：无法实现"环境光是蓝色，但漫反射是红色"的效果

**Lesson 9 的解决方案：**
```glsl
// 每个光照分量可以使用不同的光源颜色
vec3 ambient = light.ambient * material.ambient;
vec3 diffuse = light.diffuse * (diff * material.diffuse);
vec3 specular = light.specular * (spec * material.specular);
```

### 3. 代码组织需求

**Lesson 8：**
- Uniform 变量分散，难以管理
- 添加新属性需要修改多个地方

**Lesson 9：**
- 使用结构体组织相关数据
- 代码更清晰，易于维护
- 易于扩展（添加新属性只需在结构体中添加）

### 4. 性能考虑

虽然 Lesson 9 的计算稍微复杂一些，但：
- 结构体在 GPU 上组织良好
- 现代 GPU 可以高效处理结构体
- 代码清晰度带来的好处远大于微小的性能开销

---

## 如何实现材质系统

### 步骤 1：定义结构体（GLSL）

在片段着色器中定义 Material 和 Light 结构体：

```glsl
// 材质属性
struct Material {
    vec3 ambient;    // 环境光反射系数
    vec3 diffuse;    // 漫反射系数
    vec3 specular;   // 镜面反射系数
    float shininess; // 高光指数
};

// 光源属性
struct Light {
    vec3 position;   // 光源位置
    vec3 ambient;    // 环境光颜色
    vec3 diffuse;    // 漫反射颜色
    vec3 specular;   // 镜面反射颜色
};
```

**为什么使用结构体？**
- 组织相关数据
- 代码更清晰
- 易于传递和管理

### 步骤 2：声明 Uniform 变量

```glsl
uniform Material material;  // 材质
uniform Light light;         // 光源
uniform vec3 viewPos;        // 相机位置
```

### 步骤 3：实现光照计算

```glsl
void main()
{
    // 1. 环境光
    vec3 ambient = light.ambient * material.ambient;
    
    // 2. 漫反射
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);
    
    // 3. 镜面反射
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);
    
    // 4. 最终颜色
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

### 步骤 4：在 C++ 中设置 Uniform 变量

```cpp
// 设置光源属性
m_materialsShader->setVec3("light.position", m_lightPos);
m_materialsShader->setVec3("light.ambient", ambientColor);
m_materialsShader->setVec3("light.diffuse", diffuseColor);
m_materialsShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

// 设置材质属性
m_materialsShader->setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
m_materialsShader->setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
m_materialsShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
m_materialsShader->setFloat("material.shininess", 32.0f);
```

**注意：**
- 使用 `"light.ambient"` 这样的点号语法访问结构体成员
- Shader 类会自动处理结构体成员的访问

---

## 详细代码解析

### 1. 片段着色器详解

#### Material 结构体

```glsl
struct Material {
    vec3 ambient;    // 环境光反射系数
    vec3 diffuse;    // 漫反射系数
    vec3 specular;    // 镜面反射系数
    float shininess; // 高光指数
};
```

**各属性的作用：**

1. **ambient（环境光反射系数）**
   - 定义物体对环境光的反射能力
   - 通常与物体的基础颜色相同
   - 示例：`(1.0, 0.5, 0.31)` - 橙色

2. **diffuse（漫反射系数）**
   - 定义物体对漫反射光的反射能力
   - 通常与物体的基础颜色相同
   - 示例：`(1.0, 0.5, 0.31)` - 橙色

3. **specular（镜面反射系数）**
   - 定义物体对镜面反射光的反射能力
   - 通常使用灰色或白色
   - 示例：`(0.5, 0.5, 0.5)` - 灰色（降低镜面反射效果）

4. **shininess（高光指数）**
   - 控制高光的大小和锐度
   - 值越大，高光越小越锐利
   - 示例：`32.0` - 中等高光

#### Light 结构体

```glsl
struct Light {
    vec3 position;   // 光源位置
    vec3 ambient;    // 环境光颜色
    vec3 diffuse;    // 漫反射颜色
    vec3 specular;   // 镜面反射颜色
};
```

**各属性的作用：**

1. **position（光源位置）**
   - 光源在世界空间中的位置
   - 用于计算光源方向

2. **ambient（环境光颜色）**
   - 光源发出的环境光颜色
   - 通常比漫反射颜色暗（乘以 0.2）

3. **diffuse（漫反射颜色）**
   - 光源发出的漫反射光颜色
   - 通常是光源的主要颜色

4. **specular（镜面反射颜色）**
   - 光源发出的镜面反射光颜色
   - 通常是白色 `(1.0, 1.0, 1.0)`

#### 光照计算详解

```glsl
// 1. 环境光
vec3 ambient = light.ambient * material.ambient;
```

**计算过程：**
```
环境光 = 光源的环境光颜色 × 材质的环境光反射系数
```

**示例：**
```
light.ambient = (0.1, 0.05, 0.03)  // 暗橙色
material.ambient = (1.0, 0.5, 0.31)  // 橙色
ambient = (0.1, 0.05, 0.03) × (1.0, 0.5, 0.31) = (0.1, 0.025, 0.0093)
```

```glsl
// 2. 漫反射
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(light.position - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = light.diffuse * (diff * material.diffuse);
```

**计算过程：**
1. 归一化法线向量
2. 计算光源方向（从片段指向光源）
3. 计算点积（法线和光源方向的夹角）
4. 使用 max 确保非负值
5. 计算漫反射：`光源的漫反射颜色 × (点积 × 材质的漫反射系数)`

**示例：**
```
light.diffuse = (0.5, 0.25, 0.15)  // 橙色光源
material.diffuse = (1.0, 0.5, 0.31)  // 橙色材质
diff = 0.8  // 假设法线和光源方向夹角较小
diffuse = (0.5, 0.25, 0.15) × (0.8 × (1.0, 0.5, 0.31))
        = (0.5, 0.25, 0.15) × (0.8, 0.4, 0.248)
        = (0.4, 0.1, 0.0372)
```

```glsl
// 3. 镜面反射
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
vec3 specular = light.specular * (spec * material.specular);
```

**计算过程：**
1. 计算视角方向（从片段指向相机）
2. 计算反射方向（光源方向关于法线的反射）
3. 计算点积（视角方向和反射方向的夹角）
4. 使用 pow 函数计算高光强度（shininess 控制锐度）
5. 计算镜面反射：`光源的镜面反射颜色 × (高光强度 × 材质的镜面反射系数)`

**示例：**
```
light.specular = (1.0, 1.0, 1.0)  // 白色
material.specular = (0.5, 0.5, 0.5)  // 灰色
spec = 0.3  // 假设视角接近反射方向
specular = (1.0, 1.0, 1.0) × (0.3 × (0.5, 0.5, 0.5))
         = (1.0, 1.0, 1.0) × (0.15, 0.15, 0.15)
         = (0.15, 0.15, 0.15)
```

```glsl
// 4. 最终颜色
vec3 result = ambient + diffuse + specular;
FragColor = vec4(result, 1.0);
```

**最终颜色 = 环境光 + 漫反射 + 镜面反射**

### 2. C++ 代码详解

#### 动态光源颜色

```cpp
// 光源属性（颜色随时间变化）
float currentTime = GetTime();
glm::vec3 lightColor;
lightColor.x = static_cast<float>(sin(currentTime * 2.0));
lightColor.y = static_cast<float>(sin(currentTime * 0.7));
lightColor.z = static_cast<float>(sin(currentTime * 1.3));
```

**说明：**
- 使用 `sin` 函数让光源颜色在 -1 到 1 之间变化
- 不同分量使用不同的频率，产生彩色变化效果
- X 分量：频率 2.0（周期约 3.14 秒）
- Y 分量：频率 0.7（周期约 8.98 秒）
- Z 分量：频率 1.3（周期约 4.83 秒）

**注意：** `sin` 函数的值在 -1 到 1 之间，但颜色值应该在 0 到 1 之间。在实际使用中，需要调整：

```cpp
// 计算环境光和漫反射颜色（降低影响）
glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);  // 降低到 0.5
glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // 再降低到 0.2
```

这样：
- `diffuseColor` 的范围是 -0.5 到 0.5（但实际使用时会被限制）
- `ambientColor` 的范围是 -0.1 到 0.1

**更好的做法：**
```cpp
// 将 sin 的值从 [-1, 1] 映射到 [0, 1]
lightColor.x = (sin(currentTime * 2.0) + 1.0f) * 0.5f;
lightColor.y = (sin(currentTime * 0.7) + 1.0f) * 0.5f;
lightColor.z = (sin(currentTime * 1.3) + 1.0f) * 0.5f;
```

#### 设置材质属性

```cpp
// 材质属性
m_materialsShader->setVec3("material.ambient", 1.0f, 0.5f, 0.31f);   // 橙色
m_materialsShader->setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);   // 橙色
m_materialsShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);   // 灰色
m_materialsShader->setFloat("material.shininess", 32.0f);              // 高光指数
```

**材质设置说明：**

1. **ambient 和 diffuse 相同**
   - 通常物体的环境光和漫反射颜色相同
   - 都使用物体的基础颜色（橙色）

2. **specular 使用灰色**
   - `(0.5, 0.5, 0.5)` 表示镜面反射系数为 0.5
   - 降低镜面反射的效果
   - 如果使用 `(1.0, 1.0, 1.0)`，镜面反射会更强

3. **shininess = 32.0**
   - 中等的高光指数
   - 值越大，高光越小越锐利
   - 值越小，高光越大越柔和

---

## 思考与总结

### 1. 材质系统的优势

#### ✅ 灵活性

**可以为不同物体设置不同的材质：**
```cpp
// 金属材质
material.ambient = (0.25, 0.25, 0.25);
material.diffuse = (0.4, 0.4, 0.4);
material.specular = (0.774597, 0.774597, 0.774597);
material.shininess = 76.8;

// 塑料材质
material.ambient = (0.0, 0.0, 0.0);
material.diffuse = (0.55, 0.55, 0.55);
material.specular = (0.70, 0.70, 0.70);
material.shininess = 32.0;

// 橡胶材质
material.ambient = (0.02, 0.02, 0.02);
material.diffuse = (0.01, 0.01, 0.01);
material.specular = (0.4, 0.4, 0.4);
material.shininess = 10.0;
```

#### ✅ 真实性

**更接近真实材质的表现：**
- 金属：高镜面反射，低漫反射
- 塑料：中等镜面反射和漫反射
- 木头：高漫反射，低镜面反射
- 布料：几乎只有漫反射

#### ✅ 代码组织

**使用结构体组织数据：**
- 相关数据集中管理
- 代码更清晰
- 易于维护和扩展

### 2. 与 Lesson 8 的本质区别

#### Lesson 8：简单但受限

```glsl
// 所有光照分量使用相同的光源颜色
vec3 ambient = ambientStrength * lightColor;
vec3 diffuse = diff * lightColor;
vec3 specular = specularStrength * spec * lightColor;
vec3 result = (ambient + diffuse + specular) * objectColor;
```

**特点：**
- 简单直接
- 所有光照分量共享同一个光源颜色
- 物体颜色在最后统一应用

#### Lesson 9：复杂但灵活

```glsl
// 每个光照分量使用独立的光源颜色和材质系数
vec3 ambient = light.ambient * material.ambient;
vec3 diffuse = light.diffuse * (diff * material.diffuse);
vec3 specular = light.specular * (spec * material.specular);
vec3 result = ambient + diffuse + specular;
```

**特点：**
- 更灵活
- 每个光照分量可以独立控制
- 材质和光源属性分离

### 3. 为什么这样设计？

#### 设计原则

1. **分离关注点（Separation of Concerns）**
   - 材质属性：物体的固有属性
   - 光源属性：光源的特性
   - 两者分离，互不影响

2. **可组合性（Composability）**
   - 同一个材质可以用于不同的光源
   - 同一个光源可以照亮不同的材质
   - 组合方式灵活

3. **可扩展性（Extensibility）**
   - 易于添加新的材质属性
   - 易于添加新的光源类型
   - 结构体可以轻松扩展

### 4. 实际应用场景

#### 场景 1：不同材质的物体

```cpp
// 渲染金属物体
SetMaterial(metalMaterial);
DrawObject(metalObject);

// 渲染塑料物体
SetMaterial(plasticMaterial);
DrawObject(plasticObject);

// 渲染木头物体
SetMaterial(woodMaterial);
DrawObject(woodObject);
```

#### 场景 2：不同颜色的光源

```cpp
// 红色光源
light.ambient = (0.1, 0.0, 0.0);
light.diffuse = (0.5, 0.0, 0.0);
light.specular = (1.0, 1.0, 1.0);

// 蓝色光源
light.ambient = (0.0, 0.0, 0.1);
light.diffuse = (0.0, 0.0, 0.5);
light.specular = (1.0, 1.0, 1.0);
```

#### 场景 3：动态光源颜色

```cpp
// 光源颜色随时间变化
lightColor.x = sin(time * 2.0);
lightColor.y = sin(time * 0.7);
lightColor.z = sin(time * 1.3);
```

### 5. 常见材质参数参考

#### 金属材质

```cpp
Material metal;
metal.ambient = glm::vec3(0.25f, 0.25f, 0.25f);
metal.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
metal.specular = glm::vec3(0.774597f, 0.774597f, 0.774597f);
metal.shininess = 76.8f;
```

**特点：**
- 高镜面反射
- 低漫反射
- 高 shininess（高光锐利）

#### 塑料材质

```cpp
Material plastic;
plastic.ambient = glm::vec3(0.0f, 0.0f, 0.0f);
plastic.diffuse = glm::vec3(0.55f, 0.55f, 0.55f);
plastic.specular = glm::vec3(0.70f, 0.70f, 0.70f);
plastic.shininess = 32.0f;
```

**特点：**
- 中等镜面反射和漫反射
- 中等 shininess

#### 橡胶材质

```cpp
Material rubber;
rubber.ambient = glm::vec3(0.02f, 0.02f, 0.02f);
rubber.diffuse = glm::vec3(0.01f, 0.01f, 0.01f);
rubber.specular = glm::vec3(0.4f, 0.4f, 0.4f);
rubber.shininess = 10.0f;
```

**特点：**
- 低反射系数
- 低 shininess（高光柔和）

### 6. 性能考虑

#### 计算复杂度

**Lesson 8：**
- 3 次向量乘法
- 1 次标量乘法
- 相对简单

**Lesson 9：**
- 3 次向量乘法（相同）
- 结构体访问（开销很小）
- 计算复杂度基本相同

**结论：** 性能差异可以忽略不计，但灵活性大大提升。

### 7. 最佳实践

#### ✅ 推荐做法

1. **使用结构体组织数据**
   ```glsl
   struct Material { ... };
   struct Light { ... };
   ```

2. **为不同材质创建预设**
   ```cpp
   Material GetMetalMaterial() { ... }
   Material GetPlasticMaterial() { ... }
   Material GetWoodMaterial() { ... }
   ```

3. **分离材质和光源属性**
   - 材质是物体的属性
   - 光源是场景的属性

4. **使用合理的默认值**
   - ambient 和 diffuse 通常相同
   - specular 通常使用灰色或白色
   - shininess 根据材质类型设置

#### ❌ 避免的做法

1. **不要硬编码材质属性**
   ```cpp
   // ❌ 不好
   vec3 ambient = 0.1 * lightColor * vec3(1.0, 0.5, 0.31);
   
   // ✅ 好
   vec3 ambient = light.ambient * material.ambient;
   ```

2. **不要混合材质和光源属性**
   ```cpp
   // ❌ 不好
   uniform vec3 materialAndLightColor;
   
   // ✅ 好
   uniform Material material;
   uniform Light light;
   ```

---

## 总结

### Lesson 9 的核心价值

1. **引入材质系统**：将物体颜色和光源颜色分离
2. **使用结构体**：更好地组织和管理数据
3. **提高灵活性**：可以为不同物体设置不同的材质属性
4. **增强真实性**：更接近真实材质的表现

### 从 Lesson 8 到 Lesson 9 的进步

| 方面 | Lesson 8 | Lesson 9 |
|------|----------|----------|
| **数据组织** | 分散的 uniform 变量 | 结构化的 Material 和 Light |
| **灵活性** | 低 | 高 |
| **真实性** | 中等 | 高 |
| **可维护性** | 中等 | 高 |
| **可扩展性** | 低 | 高 |

### 关键收获

1. ✅ **理解了材质系统的重要性**：为什么需要分离材质和光源属性
2. ✅ **掌握了结构体的使用**：如何在 GLSL 中定义和使用结构体
3. ✅ **学会了材质属性的设置**：如何为不同材质设置合适的参数
4. ✅ **理解了光照计算的改进**：从简单乘法到独立控制每个分量

### 下一步学习方向

1. **光照贴图（Light Maps）**
   - 使用纹理控制材质属性
   - 漫反射贴图、镜面反射贴图

2. **多光源系统**
   - 多个点光源
   - 方向光、聚光灯

3. **基于物理的渲染（PBR）**
   - 更真实的光照模型
   - 能量守恒
   - 更复杂的材质属性

---

## 实验建议

### 实验 1：改变材质属性

```cpp
// 尝试不同的材质属性
material.ambient = (1.0, 0.0, 0.0);   // 红色
material.diffuse = (0.0, 1.0, 0.0);   // 绿色
material.specular = (0.0, 0.0, 1.0);  // 蓝色
```

观察效果的变化！

### 实验 2：改变高光指数

```cpp
material.shininess = 8.0;   // 大而柔和的高光
material.shininess = 32.0;  // 中等高光
material.shininess = 128.0; // 小而锐利的高光
```

### 实验 3：改变镜面反射系数

```cpp
material.specular = (0.0, 0.0, 0.0);  // 无镜面反射
material.specular = (0.5, 0.5, 0.5);  // 中等镜面反射
material.specular = (1.0, 1.0, 1.0);  // 强镜面反射
```

### 实验 4：创建不同材质的预设

```cpp
Material CreateMetalMaterial() {
    Material m;
    m.ambient = glm::vec3(0.25f, 0.25f, 0.25f);
    m.diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    m.specular = glm::vec3(0.774597f, 0.774597f, 0.774597f);
    m.shininess = 76.8f;
    return m;
}
```

---

## 常见问题

### Q1: 为什么 ambient 和 diffuse 通常相同？

**A:** 在大多数情况下，物体的环境光反射和漫反射颜色是相同的，因为它们都代表物体的基础颜色。但在某些特殊情况下（例如某些特殊材质），它们可能不同。

### Q2: specular 为什么使用灰色而不是物体的颜色？

**A:** 镜面反射通常产生白色或灰色的高光，而不是物体的颜色。这是因为镜面反射模拟的是光线在光滑表面的反射，而不是物体本身的颜色。使用灰色可以降低镜面反射的强度，使其更自然。

### Q3: shininess 的值应该如何选择？

**A:** 
- **低值（8-16）**：大而柔和的高光，适合橡胶、布料等
- **中值（32-64）**：中等高光，适合塑料、木头等
- **高值（128-256）**：小而锐利的高光，适合金属、玻璃等

### Q4: 为什么光源的 ambient 和 diffuse 颜色不同？

**A:** 在 Lesson 9 的示例中，ambient 和 diffuse 颜色不同是为了展示材质系统的灵活性。在实际应用中：
- ambient 通常是 diffuse 的 0.2 倍（更暗）
- 这样可以模拟环境光通常比直接光照暗的效果

---

## 代码对比示例

### Lesson 8 的完整光照计算

```glsl
// 环境光
float ambientStrength = 0.1;
vec3 ambient = ambientStrength * lightColor;

// 漫反射
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;

// 镜面反射
float specularStrength = 0.5;
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
vec3 specular = specularStrength * spec * lightColor;

// 最终颜色
vec3 result = (ambient + diffuse + specular) * objectColor;
FragColor = vec4(result, 1.0);
```

### Lesson 9 的完整光照计算

```glsl
// 环境光
vec3 ambient = light.ambient * material.ambient;

// 漫反射
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(light.position - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = light.diffuse * (diff * material.diffuse);

// 镜面反射
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
vec3 specular = light.specular * (spec * material.specular);

// 最终颜色
vec3 result = ambient + diffuse + specular;
FragColor = vec4(result, 1.0);
```

**关键区别：**
- Lesson 8：所有光照分量共享 `lightColor`，最后统一乘以 `objectColor`
- Lesson 9：每个光照分量使用独立的光源颜色和材质系数

---

## 最终总结

Lesson 9 的材质系统是光照系统的重要进步：

1. **从简单到复杂**：从单一颜色到结构化的材质和光源属性
2. **从固定到灵活**：从硬编码到可配置的材质参数
3. **从单一到多样**：从一种材质到多种材质支持
4. **从基础到进阶**：为学习更高级的渲染技术打下基础

**核心思想：**
- 材质定义物体如何反射光线
- 光源定义光线本身的属性
- 两者分离，灵活组合

这就是材质系统的精髓！

