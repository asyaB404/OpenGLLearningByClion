# Lesson 10: 光照贴图（Lighting Maps）详解

## 📚 目录
- [概述](#概述)
- [核心概念](#核心概念)
- [与 Lesson 9 的对比](#与-lesson-9-的对比)
- [为什么需要光照贴图](#为什么需要光照贴图)
- [如何实现光照贴图](#如何实现光照贴图)
- [详细代码解析](#详细代码解析)
- [思考与总结](#思考与总结)

---

## 概述

Lesson 10 引入了**光照贴图（Lighting Maps）**，这是材质系统的重要扩展。与 Lesson 9 相比，Lesson 10 使用**纹理贴图**来控制材质的漫反射和镜面反射属性，而不是使用固定的颜色值。这使得物体表面可以有不同的材质属性，实现更真实和丰富的视觉效果。

### 核心改进

1. **从固定颜色到纹理贴图**：使用纹理图片定义材质属性
2. **漫反射贴图（Diffuse Map）**：定义物体的颜色
3. **镜面反射贴图（Specular Map）**：定义物体的镜面反射强度
4. **顶点数据扩展**：添加纹理坐标属性

---

## 核心概念

### 1. 漫反射贴图（Diffuse Map）

**定义：** 定义物体表面每个像素的颜色，类似于传统的纹理贴图。

**作用：**
- 替代 `material.diffuse` 的固定颜色值
- 为物体表面提供详细的颜色信息
- 可以表现复杂的表面图案和细节

**示例：**
- `container2.png` - 一个容器的图片，定义了容器的颜色和图案

### 2. 镜面反射贴图（Specular Map）

**定义：** 定义物体表面每个像素的镜面反射强度。

**作用：**
- 替代 `material.specular` 的固定颜色值
- 控制哪些区域有高光，哪些区域没有
- 可以表现表面的光滑程度变化

**示例：**
- `container2_specular.png` - 定义容器哪些部分有高光（通常是边缘和金属部分）

### 3. 纹理坐标（Texture Coordinates）

**定义：** 每个顶点都有纹理坐标，用于从纹理中采样颜色。

**作用：**
- 将纹理映射到物体表面
- 定义纹理在物体上的位置和方向

---

## 与 Lesson 9 的对比

### 对比表格

| 特性 | Lesson 9 | Lesson 10 |
|------|----------|-----------|
| **材质属性类型** | vec3 颜色值 | sampler2D 纹理贴图 |
| **顶点数据格式** | 位置(3) + 法线(3) = 6个float | 位置(3) + 法线(3) + 纹理坐标(2) = 8个float |
| **纹理坐标** | ❌ 无 | ✅ 有 |
| **材质属性来源** | 固定颜色值 | 纹理贴图采样 |
| **表面细节** | 单一颜色 | 复杂图案和细节 |
| **灵活性** | 低（整个表面相同） | 高（不同区域不同） |
| **真实性** | 中等 | 高（更接近真实物体） |

### 详细对比

#### 1. 顶点数据格式

**Lesson 9：位置 + 法线**

```cpp
// 顶点数据：位置(3) + 法线(3) = 6个float
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  // 位置 + 法线
    // ...
};

// 顶点属性设置
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);      // 位置
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 法线
```

**Lesson 10：位置 + 法线 + 纹理坐标**

```cpp
// 顶点数据：位置(3) + 法线(3) + 纹理坐标(2) = 8个float
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  // 位置 + 法线 + 纹理坐标
    // ...
};

// 顶点属性设置
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);      // 位置
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // 法线
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // 纹理坐标
```

**关键区别：**
- Lesson 10 增加了纹理坐标属性（location = 2）
- 步长从 `6 * sizeof(float)` 变为 `8 * sizeof(float)`

#### 2. Material 结构体

**Lesson 9：使用 vec3 颜色值**

```glsl
struct Material {
    vec3 ambient;    // 环境光反射系数（固定颜色）
    vec3 diffuse;    // 漫反射系数（固定颜色）
    vec3 specular;   // 镜面反射系数（固定颜色）
    float shininess; // 高光指数
};
```

**Lesson 10：使用 sampler2D 纹理贴图**

```glsl
struct Material {
    sampler2D diffuse;   // 漫反射贴图（纹理）
    sampler2D specular;  // 镜面反射贴图（纹理）
    float shininess;     // 高光指数
};
```

**关键区别：**
- `ambient` 被移除（通常使用 `diffuse` 贴图）
- `diffuse` 从 `vec3` 变为 `sampler2D`
- `specular` 从 `vec3` 变为 `sampler2D`

#### 3. 光照计算

**Lesson 9：使用固定颜色值**

```glsl
// 环境光
vec3 ambient = light.ambient * material.ambient;

// 漫反射
vec3 diffuse = light.diffuse * (diff * material.diffuse);

// 镜面反射
vec3 specular = light.specular * (spec * material.specular);
```

**Lesson 10：从纹理中采样**

```glsl
// 环境光（从漫反射贴图采样）
vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

// 漫反射（从漫反射贴图采样）
vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoord)));

// 镜面反射（从镜面反射贴图采样）
vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));
```

**关键区别：**
- 使用 `texture()` 函数从纹理中采样颜色
- 每个像素的颜色来自纹理贴图，而不是固定值
- 不同区域可以有不同的颜色和反射强度

#### 4. C++ 代码设置

**Lesson 9：设置颜色值**

```cpp
// 材质属性
m_materialsShader->setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
m_materialsShader->setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
m_materialsShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
m_materialsShader->setFloat("material.shininess", 32.0f);
```

**Lesson 10：设置纹理单元**

```cpp
// 配置着色器（设置纹理单元）
m_lightingShader->use();
m_lightingShader->setInt("material.diffuse", 0);   // 纹理单元 0
m_lightingShader->setInt("material.specular", 1);  // 纹理单元 1

// 加载纹理
m_diffuseMap = loadTexture("container2.png");
m_specularMap = loadTexture("container2_specular.png");

// 在渲染时绑定纹理
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, m_diffuseMap);
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, m_specularMap);
```

**关键区别：**
- 使用 `setInt()` 设置纹理单元索引
- 需要加载纹理文件
- 需要在渲染时绑定纹理到对应的纹理单元

---

## 为什么需要光照贴图？

### 1. 真实世界的需求

在真实世界中，物体表面的材质属性不是均匀的：

- **容器**：边缘是金属（高镜面反射），中间是塑料（低镜面反射）
- **木头**：纹理部分和光滑部分的反射不同
- **石头**：不同区域的光滑程度不同

**Lesson 9 的问题：**
- 整个表面使用相同的材质属性
- 无法表现表面的细节变化

**Lesson 10 的解决方案：**
- 使用纹理贴图定义每个像素的材质属性
- 可以表现复杂的表面细节

### 2. 视觉效果需求

**Lesson 9：**
```
立方体：
- 所有面都是相同的颜色
- 所有面的高光强度相同
- 看起来是"平的"
```

**Lesson 10：**
```
立方体：
- 不同区域有不同的颜色（来自纹理）
- 不同区域有不同的高光强度（来自镜面反射贴图）
- 看起来更真实，有细节
```

### 3. 灵活性需求

**Lesson 9：**
- 需要为每个不同的物体创建不同的材质
- 无法在同一物体上表现不同的材质区域

**Lesson 10：**
- 使用纹理贴图，可以在同一物体上表现不同的材质区域
- 更灵活，更容易创建复杂的视觉效果

### 4. 艺术创作需求

**光照贴图允许艺术家：**
- 在 Photoshop 等工具中绘制材质属性
- 精确控制每个像素的材质属性
- 创建复杂的视觉效果

---

## 如何实现光照贴图

### 步骤 1：扩展顶点数据

添加纹理坐标到顶点数据：

```cpp
// 顶点数据：位置(3) + 法线(3) + 纹理坐标(2) = 8个float
float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
    // 位置(3)     法线(3)          纹理坐标(2)
    // ...
};
```

### 步骤 2：设置纹理坐标属性

```cpp
// 纹理坐标属性（location = 2）
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
glEnableVertexAttribArray(2);
```

### 步骤 3：修改 Material 结构体

```glsl
struct Material {
    sampler2D diffuse;   // 漫反射贴图
    sampler2D specular;  // 镜面反射贴图
    float shininess;     // 高光指数
};
```

### 步骤 4：在片段着色器中采样纹理

```glsl
// 从纹理中采样颜色
vec3 diffuseColor = vec3(texture(material.diffuse, TexCoord));
vec3 specularColor = vec3(texture(material.specular, TexCoord));

// 使用采样后的颜色计算光照
vec3 ambient = light.ambient * diffuseColor;
vec3 diffuse = light.diffuse * (diff * diffuseColor);
vec3 specular = light.specular * (spec * specularColor);
```

### 步骤 5：加载纹理并绑定

```cpp
// 加载纹理
unsigned int diffuseMap = loadTexture("container2.png");
unsigned int specularMap = loadTexture("container2_specular.png");

// 设置纹理单元
shader.setInt("material.diffuse", 0);
shader.setInt("material.specular", 1);

// 绑定纹理
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, diffuseMap);
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, specularMap);
```

---

## 详细代码解析

### 1. 顶点着色器

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;      // 位置
layout (location = 1) in vec3 aNormal;  // 法线
layout (location = 2) in vec2 aTexCoord;// 纹理坐标（新增）

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoord;                       // 输出纹理坐标（新增）

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;                // 传递纹理坐标
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
```

**关键点：**
- 输入纹理坐标 `aTexCoord`
- 输出纹理坐标 `TexCoord` 到片段着色器
- 纹理坐标不需要变换，直接传递

### 2. 片段着色器

```glsl
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;                        // 输入纹理坐标

uniform vec3 viewPos;

struct Material {
    sampler2D diffuse;                   // 漫反射贴图
    sampler2D specular;                   // 镜面反射贴图
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main()
{
    // 环境光：从漫反射贴图采样
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));

    // 漫反射：从漫反射贴图采样
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoord)));

    // 镜面反射：从镜面反射贴图采样
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));

    // 最终颜色
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
```

**关键点：**

1. **纹理采样**
   ```glsl
   texture(material.diffuse, TexCoord)
   ```
   - `material.diffuse` 是 `sampler2D` 类型
   - `TexCoord` 是纹理坐标 `(u, v)`
   - 返回 `vec4`，需要转换为 `vec3`

2. **环境光使用漫反射贴图**
   ```glsl
   vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
   ```
   - 通常环境光也使用漫反射贴图
   - 这样可以保持一致性

3. **镜面反射贴图的作用**
   ```glsl
   vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));
   ```
   - 镜面反射贴图控制哪些区域有高光
   - 白色区域 = 有高光
   - 黑色区域 = 无高光
   - 灰色区域 = 中等高光

### 3. C++ 代码详解

#### 加载纹理

```cpp
// 加载漫反射贴图
std::string diffusePath = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/container2.png";
m_diffuseMap = loadTexture(diffusePath.c_str());

// 加载镜面反射贴图
std::string specularPath = std::string(PROJECT_ROOT) + "/engine/assets/texture/lesson/container2_specular.png";
m_specularMap = loadTexture(specularPath.c_str());
```

#### 设置纹理单元

```cpp
// 配置着色器（设置纹理单元）
m_lightingShader->use();
m_lightingShader->setInt("material.diffuse", 0);   // 纹理单元 0
m_lightingShader->setInt("material.specular", 1);  // 纹理单元 1
```

**纹理单元（Texture Unit）：**
- OpenGL 支持多个纹理同时使用
- 每个纹理单元对应一个纹理
- `GL_TEXTURE0` 对应纹理单元 0
- `GL_TEXTURE1` 对应纹理单元 1

#### 绑定纹理

```cpp
// 绑定漫反射贴图到纹理单元 0
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, m_diffuseMap);

// 绑定镜面反射贴图到纹理单元 1
glActiveTexture(GL_TEXTURE1);
glBindTexture(GL_TEXTURE_2D, m_specularMap);
```

**执行顺序：**
1. 激活纹理单元（`glActiveTexture`）
2. 绑定纹理（`glBindTexture`）
3. 着色器中的 `sampler2D` 会自动从对应的纹理单元读取

---

## 思考与总结

### 1. 光照贴图的优势

#### ✅ 真实性

**使用纹理贴图可以表现：**
- 复杂的表面图案
- 不同区域的材质差异
- 细节和纹理

**示例：**
- 容器的边缘是金属（高光），中间是塑料（低光）
- 木头的纹理部分和光滑部分
- 石头的不同区域

#### ✅ 灵活性

**可以为不同区域设置不同的材质属性：**
- 同一物体上可以有多种材质
- 不需要为每个区域创建不同的物体
- 更容易创建复杂的视觉效果

#### ✅ 艺术创作

**艺术家可以在 Photoshop 中：**
- 绘制漫反射贴图（定义颜色）
- 绘制镜面反射贴图（定义高光区域）
- 精确控制每个像素的材质属性

### 2. 与 Lesson 9 的本质区别

#### Lesson 9：固定颜色值

```glsl
// 所有像素使用相同的颜色
vec3 ambient = light.ambient * material.ambient;  // 固定值
vec3 diffuse = light.diffuse * (diff * material.diffuse);  // 固定值
vec3 specular = light.specular * (spec * material.specular);  // 固定值
```

**特点：**
- 简单直接
- 整个表面使用相同的材质属性
- 无法表现表面细节

#### Lesson 10：纹理贴图

```glsl
// 每个像素从纹理中采样颜色
vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord));
vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoord)));
vec3 specular = light.specular * (spec * vec3(texture(material.specular, TexCoord)));
```

**特点：**
- 更复杂但更灵活
- 每个像素可以有不同的材质属性
- 可以表现复杂的表面细节

### 3. 为什么这样设计？

#### 设计原则

1. **分离关注点**
   - 漫反射贴图：定义颜色
   - 镜面反射贴图：定义高光强度
   - 两者分离，互不影响

2. **可组合性**
   - 可以为同一物体使用不同的贴图组合
   - 可以混合使用固定颜色和纹理贴图

3. **可扩展性**
   - 易于添加新的贴图类型（例如：法线贴图、粗糙度贴图）
   - 结构体可以轻松扩展

### 4. 纹理贴图的作用

#### 漫反射贴图（Diffuse Map）

**作用：**
- 定义物体的颜色
- 表现表面的图案和细节
- 类似于传统的纹理贴图

**示例：**
- `container2.png` - 容器的图片，定义了容器的颜色和图案

#### 镜面反射贴图（Specular Map）

**作用：**
- 定义物体的镜面反射强度
- 控制哪些区域有高光
- 表现表面的光滑程度

**示例：**
- `container2_specular.png` - 定义容器哪些部分有高光
  - 白色区域 = 有高光（金属边缘）
  - 黑色区域 = 无高光（塑料部分）
  - 灰色区域 = 中等高光

### 5. 常见贴图类型

#### 基础贴图（Lesson 10）

1. **漫反射贴图（Diffuse Map）**
   - 定义物体的颜色
   - 也称为 Albedo Map 或 Base Color Map

2. **镜面反射贴图（Specular Map）**
   - 定义镜面反射强度
   - 控制高光区域

#### 高级贴图（后续课程）

3. **法线贴图（Normal Map）**
   - 定义表面的凹凸细节
   - 不需要额外的顶点

4. **粗糙度贴图（Roughness Map）**
   - 定义表面的粗糙程度
   - 用于 PBR 渲染

5. **环境光遮蔽贴图（AO Map）**
   - 定义环境光的遮蔽效果
   - 增加细节和深度

### 6. 性能考虑

#### 纹理采样开销

**每次纹理采样：**
- 需要访问 GPU 内存
- 需要插值计算（如果使用线性过滤）
- 有一定的性能开销

**优化建议：**
- 使用合适的纹理分辨率（不要太大）
- 使用 Mipmap 减少远处纹理的采样开销
- 合理使用纹理压缩格式

#### 多纹理使用

**Lesson 10 使用 2 个纹理：**
- 漫反射贴图：纹理单元 0
- 镜面反射贴图：纹理单元 1

**现代 GPU 支持：**
- 同时使用多个纹理（通常 16+ 个纹理单元）
- 性能开销可以忽略不计

---

## 详细代码解析

### 1. 顶点数据格式详解

```cpp
float vertices[] = {
    // 位置(3)     法线(3)          纹理坐标(2)
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
    //  ^^^^^^^^    ^^^^^^^^^^^^     ^^^^^^^^
    //  位置(xyz)   法线(nx,ny,nz)   纹理坐标(u,v)
};
```

**数据布局：**
```
每个顶点：8 个 float = 32 字节
[位置(12字节)] [法线(12字节)] [纹理坐标(8字节)]
```

**纹理坐标说明：**
- `(0.0, 0.0)` - 纹理左下角
- `(1.0, 0.0)` - 纹理右下角
- `(1.0, 1.0)` - 纹理右上角
- `(0.0, 1.0)` - 纹理左上角

### 2. 纹理采样详解

#### texture() 函数

```glsl
texture(sampler2D sampler, vec2 coord)
```

**参数：**
- `sampler`：纹理采样器（`sampler2D` 类型）
- `coord`：纹理坐标 `(u, v)`，范围通常是 `[0.0, 1.0]`

**返回值：**
- `vec4`：RGBA 颜色值

**使用示例：**
```glsl
vec4 texColor = texture(material.diffuse, TexCoord);
vec3 color = vec3(texColor);  // 转换为 vec3（忽略 Alpha）
```

#### 纹理坐标插值

**关键理解：**
- 纹理坐标在顶点着色器中设置
- OpenGL 会自动在三角形内部插值纹理坐标
- 片段着色器接收插值后的纹理坐标

**示例：**
```
顶点 0: TexCoord = (0.0, 0.0)
顶点 1: TexCoord = (1.0, 0.0)
顶点 2: TexCoord = (0.5, 1.0)

三角形内部的像素：
- 靠近顶点 0 的像素：TexCoord ≈ (0.0, 0.0)
- 靠近顶点 1 的像素：TexCoord ≈ (1.0, 0.0)
- 三角形中心的像素：TexCoord ≈ (0.5, 0.33)
```

### 3. 纹理单元详解

#### 什么是纹理单元？

**纹理单元（Texture Unit）：**
- OpenGL 支持同时使用多个纹理
- 每个纹理单元对应一个纹理
- 默认有 16 个纹理单元（`GL_TEXTURE0` 到 `GL_TEXTURE15`）

#### 如何使用纹理单元？

```cpp
// 1. 设置着色器中的纹理单元索引
shader.setInt("material.diffuse", 0);   // 告诉着色器使用纹理单元 0
shader.setInt("material.specular", 1);  // 告诉着色器使用纹理单元 1

// 2. 激活纹理单元
glActiveTexture(GL_TEXTURE0);  // 激活纹理单元 0

// 3. 绑定纹理到当前激活的纹理单元
glBindTexture(GL_TEXTURE_2D, diffuseMap);  // 绑定到纹理单元 0

// 4. 激活下一个纹理单元
glActiveTexture(GL_TEXTURE1);  // 激活纹理单元 1

// 5. 绑定下一个纹理
glBindTexture(GL_TEXTURE_2D, specularMap);  // 绑定到纹理单元 1
```

**执行流程：**
1. 着色器中的 `sampler2D` 知道使用哪个纹理单元（通过 `setInt` 设置）
2. `glActiveTexture` 激活对应的纹理单元
3. `glBindTexture` 将纹理绑定到当前激活的纹理单元
4. 片段着色器中的 `texture()` 函数从对应的纹理单元读取

### 4. 镜面反射贴图的工作原理

#### 镜面反射贴图的作用

**镜面反射贴图定义：**
- 每个像素的镜面反射强度
- 白色 = 高镜面反射（有高光）
- 黑色 = 低镜面反射（无高光）
- 灰色 = 中等镜面反射

**计算过程：**
```glsl
// 1. 从镜面反射贴图采样
vec3 specularColor = vec3(texture(material.specular, TexCoord));
// 例如：specularColor = (1.0, 1.0, 1.0) - 白色（有高光）
// 例如：specularColor = (0.0, 0.0, 0.0) - 黑色（无高光）

// 2. 计算镜面反射强度
float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
// 例如：spec = 0.5（中等强度）

// 3. 计算最终镜面反射
vec3 specular = light.specular * (spec * specularColor);
// 如果 specularColor = (1.0, 1.0, 1.0)，specular = light.specular * 0.5（有高光）
// 如果 specularColor = (0.0, 0.0, 0.0)，specular = (0.0, 0.0, 0.0)（无高光）
```

**实际效果：**
- 白色区域：有高光，看起来光滑
- 黑色区域：无高光，看起来粗糙
- 灰色区域：中等高光

---

## 思考与总结

### 1. 光照贴图的优势

#### ✅ 真实性

**可以表现：**
- 复杂的表面图案
- 不同区域的材质差异
- 细节和纹理

#### ✅ 灵活性

**可以为不同区域设置不同的材质属性：**
- 同一物体上可以有多种材质
- 不需要为每个区域创建不同的物体

#### ✅ 艺术创作

**艺术家可以在 Photoshop 中：**
- 绘制漫反射贴图
- 绘制镜面反射贴图
- 精确控制每个像素的材质属性

### 2. 与 Lesson 9 的本质区别

#### Lesson 9：固定颜色值

- 整个表面使用相同的材质属性
- 无法表现表面细节
- 简单但受限

#### Lesson 10：纹理贴图

- 每个像素可以有不同的材质属性
- 可以表现复杂的表面细节
- 复杂但灵活

### 3. 为什么这样设计？

#### 设计原则

1. **分离关注点**
   - 漫反射贴图：定义颜色
   - 镜面反射贴图：定义高光强度

2. **可组合性**
   - 可以为同一物体使用不同的贴图组合
   - 可以混合使用固定颜色和纹理贴图

3. **可扩展性**
   - 易于添加新的贴图类型
   - 结构体可以轻松扩展

### 4. 实际应用场景

#### 场景 1：容器

```
漫反射贴图：容器的颜色和图案
镜面反射贴图：边缘是白色（金属，有高光），中间是黑色（塑料，无高光）
```

#### 场景 2：木头

```
漫反射贴图：木头的纹理和颜色
镜面反射贴图：纹理部分是黑色（粗糙），光滑部分是白色（有高光）
```

#### 场景 3：石头

```
漫反射贴图：石头的颜色和纹理
镜面反射贴图：不同区域有不同的光滑程度
```

### 5. 常见问题

#### Q1: 为什么环境光也使用漫反射贴图？

**A:** 环境光通常使用漫反射贴图，因为：
- 环境光代表间接光照，应该使用物体的基础颜色
- 保持一致性，避免颜色不匹配
- 简化计算

#### Q2: 镜面反射贴图为什么是黑白的？

**A:** 镜面反射贴图通常使用灰度图（黑白），因为：
- 它只定义强度，不定义颜色
- 镜面反射的颜色通常来自光源，而不是物体
- 使用灰度图更直观（白色=有高光，黑色=无高光）

#### Q3: 可以只使用漫反射贴图，不使用镜面反射贴图吗？

**A:** 可以！可以：
- 使用固定的镜面反射颜色值
- 或者使用漫反射贴图作为镜面反射贴图
- 或者使用纯白色（所有区域都有高光）

#### Q4: 纹理坐标的范围是什么？

**A:** 通常是 `[0.0, 1.0]`，但可以超出：
- `[0.0, 1.0]`：正常范围
- `< 0.0` 或 `> 1.0`：根据纹理包装模式（`GL_REPEAT`、`GL_CLAMP` 等）处理

---

## 总结

### Lesson 10 的核心价值

1. **引入光照贴图**：使用纹理贴图控制材质属性
2. **漫反射贴图**：定义物体的颜色和图案
3. **镜面反射贴图**：定义物体的高光区域
4. **提高真实性**：可以表现复杂的表面细节

### 从 Lesson 9 到 Lesson 10 的进步

| 方面 | Lesson 9 | Lesson 10 |
|------|----------|-----------|
| **材质属性** | 固定颜色值 | 纹理贴图 |
| **表面细节** | 单一颜色 | 复杂图案 |
| **灵活性** | 低 | 高 |
| **真实性** | 中等 | 高 |
| **艺术创作** | 受限 | 灵活 |

### 关键收获

1. ✅ **理解了光照贴图的作用**：为什么需要纹理贴图控制材质属性
2. ✅ **掌握了纹理采样**：如何在着色器中使用纹理
3. ✅ **学会了纹理单元的使用**：如何同时使用多个纹理
4. ✅ **理解了镜面反射贴图**：如何控制高光区域

### 下一步学习方向

1. **法线贴图（Normal Maps）**
   - 增加表面细节
   - 不需要额外的顶点

2. **更多贴图类型**
   - 粗糙度贴图
   - 环境光遮蔽贴图
   - 自发光贴图

3. **基于物理的渲染（PBR）**
   - 更真实的光照模型
   - 更复杂的材质系统

---

## 实验建议

### 实验 1：改变镜面反射贴图

尝试使用不同的镜面反射贴图：
- 纯白色：所有区域都有高光
- 纯黑色：所有区域都无高光
- 渐变：从白色到黑色的渐变

### 实验 2：只使用漫反射贴图

```glsl
// 镜面反射也使用漫反射贴图
vec3 specular = light.specular * (spec * vec3(texture(material.diffuse, TexCoord)));
```

### 实验 3：混合使用固定颜色和纹理

```glsl
// 环境光使用固定颜色
vec3 ambient = light.ambient * material.ambient;

// 漫反射使用纹理
vec3 diffuse = light.diffuse * (diff * vec3(texture(material.diffuse, TexCoord)));
```

---

## 最终总结

Lesson 10 的光照贴图是材质系统的重要扩展：

1. **从固定到纹理**：从固定颜色值到纹理贴图
2. **从简单到复杂**：从单一颜色到复杂图案
3. **从均匀到变化**：从整个表面相同到不同区域不同
4. **从基础到进阶**：为学习更高级的渲染技术打下基础

**核心思想：**
- 使用纹理贴图定义材质属性
- 每个像素可以有不同的材质属性
- 可以表现复杂的表面细节

这就是光照贴图的精髓！

