# Lesson 17: 立方体贴图和天空盒总结

## 📚 目录
- [概述](#概述)
- [立方体贴图基础](#立方体贴图基础)
- [Lesson 17: 立方体贴图和天空盒实现](#lesson-17-立方体贴图和天空盒实现)
- [立方体贴图详解](#立方体贴图详解)
- [天空盒详解](#天空盒详解)
- [环境映射详解](#环境映射详解)
- [反射效果详解](#反射效果详解)
- [着色器详解](#着色器详解)
- [渲染流程详解](#渲染流程详解)
- [常见问题](#常见问题)
- [立方体贴图 vs 其他技术](#立方体贴图-vs-其他技术)
- [实际应用](#实际应用)
- [总结](#总结)

---

## 概述

Lesson 17 介绍了 **立方体贴图（Cubemaps）** 和 **天空盒（Skybox）** 的概念和使用方法。通过立方体贴图，我们可以实现环境映射、反射效果和天空盒等高级渲染技术。

### 学习目标

- ✅ 理解立方体贴图的概念和作用
- ✅ 掌握如何加载和使用立方体贴图
- ✅ 学会实现天空盒
- ✅ 理解环境映射的工作原理
- ✅ 实现反射效果

---

## 立方体贴图基础

### 什么是立方体贴图？

**立方体贴图（Cubemap）** 是一种特殊的纹理，由 6 个 2D 纹理组成，形成一个立方体的 6 个面：

- **右面（Right / +X）**：`GL_TEXTURE_CUBE_MAP_POSITIVE_X`
- **左面（Left / -X）**：`GL_TEXTURE_CUBE_MAP_NEGATIVE_X`
- **上面（Top / +Y）**：`GL_TEXTURE_CUBE_MAP_POSITIVE_Y`
- **下面（Bottom / -Y）**：`GL_TEXTURE_CUBE_MAP_NEGATIVE_Y`
- **前面（Front / +Z）**：`GL_TEXTURE_CUBE_MAP_POSITIVE_Z`
- **后面（Back / -Z）**：`GL_TEXTURE_CUBE_MAP_NEGATIVE_Z`

### 为什么需要立方体贴图？

立方体贴图的主要用途：

1. **天空盒（Skybox）**：渲染天空背景
2. **环境映射（Environment Mapping）**：反射和折射效果
3. **IBL（Image-Based Lighting）**：基于图像的光照
4. **阴影映射**：点光源阴影

### 立方体贴图的工作原理

立方体贴图使用 **3D 方向向量** 进行采样：

```
采样方向 = normalize(位置 - 相机位置)
颜色 = texture(cubemap, 采样方向)
```

**关键**：方向向量从立方体中心指向外部，OpenGL 会自动选择对应的面进行采样。

---

## Lesson 17: 立方体贴图和天空盒实现

### 功能特点

- **反射立方体**：立方体表面反射天空盒环境
- **天空盒**：渲染天空背景
- **相机系统**：可以自由移动相机观察效果
- **环境映射**：使用立方体贴图实现反射效果

### 关键代码

#### 加载立方体贴图

```cpp
static unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, 
                        width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    
    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
```

#### 立方体贴图文件顺序

```cpp
std::vector<std::string> faces
{
    "right.jpg",   // GL_TEXTURE_CUBE_MAP_POSITIVE_X
    "left.jpg",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
    "top.jpg",     // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
    "bottom.jpg",  // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
    "front.jpg",   // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
    "back.jpg",    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};
```

**重要**：文件顺序必须正确，否则立方体贴图会显示错误。

### 视觉效果

- **反射立方体**：立方体表面反射天空盒环境
- **天空盒**：天空背景，始终显示在场景最远处
- **环境映射**：立方体根据视角反射不同的天空盒部分

### 控制说明

- **WASD**：移动相机
- **鼠标移动**：旋转视角
- **滚轮**：缩放视野
- **ESC**：退出程序

---

## 立方体贴图详解

### 立方体贴图的坐标系统

立方体贴图使用右手坐标系：

```
        +Y (Top)
         |
         |
         |
         +-------+X (Right)
        /
       /
      /
     +Z (Front)
```

**面的定义**：
- **+X (Right)**：立方体的右面
- **-X (Left)**：立方体的左面
- **+Y (Top)**：立方体的上面
- **-Y (Bottom)**：立方体的下面
- **+Z (Front)**：立方体的前面
- **-Z (Back)**：立方体的后面

### 立方体贴图的采样

立方体贴图使用 **方向向量** 进行采样：

```glsl
vec3 direction = normalize(Position - cameraPos);
vec3 color = texture(cubemap, direction).rgb;
```

**工作原理**：
1. 计算从立方体中心到采样点的方向向量
2. 方向向量从立方体中心指向外部
3. OpenGL 自动选择对应的面进行采样
4. 返回该方向上的颜色值

### 立方体贴图的纹理参数

立方体贴图必须使用特定的纹理参数：

```cpp
// 过滤模式
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// 包装模式（必须使用 GL_CLAMP_TO_EDGE）
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
```

**为什么必须使用 GL_CLAMP_TO_EDGE？**

立方体贴图的边缘必须无缝连接。使用 `GL_CLAMP_TO_EDGE` 可以确保边缘像素正确采样，避免接缝问题。

### 立方体贴图的格式

立方体贴图支持多种格式：

| 格式 | 说明 | 使用场景 |
|------|------|----------|
| `GL_RGB` | 24 位颜色 | 标准天空盒 |
| `GL_RGBA` | 32 位颜色 | 带透明度的天空盒 |
| `GL_RGB16F` | 16 位浮点 | HDR 天空盒 |
| `GL_RGB32F` | 32 位浮点 | 高精度 HDR 天空盒 |

---

## 天空盒详解

### 什么是天空盒？

**天空盒（Skybox）** 是一个巨大的立方体，包围整个场景，用于渲染天空背景。它始终显示在场景的最远处，给场景提供环境感。

### 天空盒的实现

#### 1. 创建天空盒立方体

天空盒使用一个巨大的立方体（通常大小为 1.0，通过视图矩阵缩放）：

```cpp
float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    // ... 其他顶点 ...
};
```

#### 2. 移除视图矩阵的平移部分

天空盒应该始终在相机位置，不受相机移动影响：

```cpp
// 移除视图矩阵的平移部分（只保留旋转）
view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
```

**说明**：
- `glm::mat3()` 提取 3x3 旋转矩阵
- `glm::mat4()` 转换回 4x4 矩阵（平移部分为 0）
- 这样天空盒会跟随相机旋转，但不会移动

#### 3. 深度测试设置

天空盒应该始终在最远处，使用特殊的深度测试：

```cpp
// 改变深度函数，使得深度测试在值相等时通过
glDepthFunc(GL_LEQUAL);

// 渲染天空盒
glDrawArrays(GL_TRIANGLES, 0, 36);

// 恢复深度函数
glDepthFunc(GL_LESS);
```

**说明**：
- `GL_LEQUAL`：当深度值 ≤ 深度缓冲值时通过
- 天空盒的深度值通常是 1.0（最远）
- 这样可以确保天空盒显示在所有物体后面

#### 4. 顶点着色器优化

天空盒的顶点着色器使用 `pos.xyww` 技巧：

```glsl
vec4 pos = projection * view * vec4(aPos, 1.0);
gl_Position = pos.xyww;
```

**说明**：
- `xyww` 将 Z 坐标设置为 W 坐标
- 经过透视除法后，Z 值变为 1.0（最远）
- 这样可以确保天空盒始终在最远处

### 天空盒的渲染顺序

天空盒应该 **最后渲染**：

1. 先渲染场景中的所有物体
2. 最后渲染天空盒

**原因**：
- 天空盒覆盖整个屏幕
- 先渲染会被其他物体覆盖
- 最后渲染可以确保显示在最远处

---

## 环境映射详解

### 什么是环境映射？

**环境映射（Environment Mapping）** 是一种使用立方体贴图模拟环境反射的技术。它可以让物体表面反射周围的环境。

### 反射效果（Reflection）

反射效果使用 **反射向量** 从立方体贴图中采样：

```glsl
// 计算入射向量（从相机到片段）
vec3 I = normalize(Position - cameraPos);

// 计算反射向量
vec3 R = reflect(I, normalize(Normal));

// 从立方体贴图中采样
vec3 color = texture(skybox, R).rgb;
```

**反射公式**：
```
R = I - 2 * dot(I, N) * N
```

其中：
- `I`：入射向量（从相机到片段）
- `N`：法线向量
- `R`：反射向量

### 折射效果（Refraction）

折射效果使用 **折射向量** 从立方体贴图中采样：

```glsl
// 计算折射向量
vec3 R = refract(I, normalize(Normal), ratio);

// 从立方体贴图中采样
vec3 color = texture(skybox, R).rgb;
```

**折射公式**：
```
R = refract(I, N, eta)
```

其中：
- `I`：入射向量
- `N`：法线向量
- `eta`：折射率（例如：空气到玻璃 = 1.0/1.5）

### 菲涅尔效果（Fresnel）

菲涅尔效果结合反射和折射，根据视角混合两者：

```glsl
// 计算菲涅尔系数
float fresnel = pow(1.0 - dot(viewDir, normal), 2.0);

// 混合反射和折射
vec3 color = mix(refractionColor, reflectionColor, fresnel);
```

**说明**：
- 视角越倾斜，反射越强
- 视角越垂直，折射越强
- 产生更真实的玻璃效果

---

## 反射效果详解

### 反射效果的实现

#### 顶点着色器

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 计算世界空间法线
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // 计算世界空间位置
    Position = vec3(model * vec4(aPos, 1.0));
    
    // 计算裁剪空间位置
    gl_Position = projection * view * vec4(Position, 1.0);
}
```

**关键点**：
- 法线需要转换到世界空间
- 位置需要转换到世界空间
- 使用 `transpose(inverse(model))` 处理非均匀缩放

#### 片段着色器

```glsl
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{
    // 计算入射向量（从相机到片段）
    vec3 I = normalize(Position - cameraPos);
    
    // 计算反射向量
    vec3 R = reflect(I, normalize(Normal));
    
    // 从立方体贴图中采样
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}
```

**关键点**：
- 使用 `reflect()` 函数计算反射向量
- 使用 `texture()` 函数从立方体贴图中采样
- 反射向量作为采样方向

### 反射效果的优化

#### 1. 法线矩阵优化

法线矩阵可以预先计算：

```cpp
glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
shader.setMat3("normalMatrix", normalMatrix);
```

```glsl
Normal = normalMatrix * aNormal;
```

#### 2. 部分反射

可以混合反射颜色和物体颜色：

```glsl
vec3 reflectionColor = texture(skybox, R).rgb;
vec3 objectColor = vec3(0.5, 0.5, 0.5);
float reflectivity = 0.8;

vec3 color = mix(objectColor, reflectionColor, reflectivity);
FragColor = vec4(color, 1.0);
```

---

## 着色器详解

### 反射着色器

#### 顶点着色器 (`6.2.cubemaps.vs`)

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Position = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(Position, 1.0);
}
```

**功能**：
- 计算世界空间法线和位置
- 用于后续的反射计算

#### 片段着色器 (`6.2.cubemaps.fs`)

```glsl
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main()
{
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}
```

**功能**：
- 计算反射向量
- 从立方体贴图中采样反射颜色

### 天空盒着色器

#### 顶点着色器 (`6.2.skybox.vs`)

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
```

**功能**：
- 直接使用顶点位置作为纹理坐标
- 使用 `xyww` 技巧确保深度值为 1.0

#### 片段着色器 (`6.2.skybox.fs`)

```glsl
#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}
```

**功能**：
- 直接从立方体贴图中采样
- 使用顶点位置作为采样方向

---

## 渲染流程详解

### 完整的渲染流程

#### 第一步：渲染反射物体

```cpp
// 使用反射着色器
m_shader->use();

// 设置矩阵
m_shader->setMat4("model", model);
m_shader->setMat4("view", view);
m_shader->setMat4("projection", projection);
m_shader->setVec3("cameraPos", m_camera.Position);

// 绑定立方体贴图
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTexture);

// 渲染立方体
glDrawArrays(GL_TRIANGLES, 0, 36);
```

**结果**：
- 立方体表面反射天空盒环境
- 根据视角显示不同的反射内容

#### 第二步：渲染天空盒

```cpp
// 改变深度函数
glDepthFunc(GL_LEQUAL);

// 使用天空盒着色器
m_skyboxShader->use();

// 移除视图矩阵的平移部分
view = glm::mat4(glm::mat3(m_camera.GetViewMatrix()));
m_skyboxShader->setMat4("view", view);
m_skyboxShader->setMat4("projection", projection);

// 绑定立方体贴图
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapTexture);

// 渲染天空盒
glDrawArrays(GL_TRIANGLES, 0, 36);

// 恢复深度函数
glDepthFunc(GL_LESS);
```

**结果**：
- 天空盒显示在场景最远处
- 跟随相机旋转，但不移动

### 渲染顺序的重要性

**正确的渲染顺序**：
1. 先渲染场景物体（反射物体）
2. 最后渲染天空盒

**原因**：
- 天空盒应该显示在所有物体后面
- 先渲染会被其他物体覆盖
- 最后渲染可以确保正确显示

---

## 常见问题

### Q1: 立方体贴图显示错误或接缝？

**A:** 可能的原因：

1. **文件顺序错误**：确保文件顺序正确
   ```cpp
   right, left, top, bottom, front, back
   ```

2. **纹理参数错误**：必须使用 `GL_CLAMP_TO_EDGE`
   ```cpp
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   ```

3. **图片尺寸不一致**：所有面的图片尺寸必须相同

### Q2: 天空盒不显示或显示错误？

**A:** 检查以下几点：

1. **视图矩阵**：必须移除平移部分
   ```cpp
   view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
   ```

2. **深度函数**：必须使用 `GL_LEQUAL`
   ```cpp
   glDepthFunc(GL_LEQUAL);
   ```

3. **渲染顺序**：天空盒必须最后渲染

4. **顶点着色器**：使用 `pos.xyww` 技巧
   ```glsl
   gl_Position = pos.xyww;
   ```

### Q3: 反射效果不正确？

**A:** 检查以下几点：

1. **法线计算**：确保法线正确转换到世界空间
   ```glsl
   Normal = mat3(transpose(inverse(model))) * aNormal;
   ```

2. **位置计算**：确保位置正确转换到世界空间
   ```glsl
   Position = vec3(model * vec4(aPos, 1.0));
   ```

3. **反射向量**：确保使用正确的反射公式
   ```glsl
   vec3 I = normalize(Position - cameraPos);
   vec3 R = reflect(I, normalize(Normal));
   ```

### Q4: 如何实现部分反射？

**A:** 混合反射颜色和物体颜色：

```glsl
vec3 reflectionColor = texture(skybox, R).rgb;
vec3 objectColor = vec3(0.5, 0.5, 0.5);
float reflectivity = 0.8;

vec3 color = mix(objectColor, reflectionColor, reflectivity);
FragColor = vec4(color, 1.0);
```

### Q5: 如何实现折射效果？

**A:** 使用 `refract()` 函数：

```glsl
// 计算折射向量
float ratio = 1.0 / 1.5;  // 空气到玻璃的折射率
vec3 R = refract(I, normalize(Normal), ratio);

// 从立方体贴图中采样
vec3 color = texture(skybox, R).rgb;
FragColor = vec4(color, 1.0);
```

### Q6: 立方体贴图的性能影响？

**A:** 立方体贴图的性能开销：

- **内存开销**：6 个纹理面（中等）
- **采样开销**：方向向量采样（很小）
- **纹理切换**：绑定立方体贴图（很小）

**优化建议**：
1. 使用合适的分辨率（不需要太高）
2. 使用压缩纹理格式
3. 避免频繁切换立方体贴图

### Q7: 如何创建 HDR 天空盒？

**A:** 使用浮点纹理：

```cpp
// 创建浮点纹理
glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 
            width, height, 0, GL_RGB, GL_FLOAT, data);
```

**优点**：
- 支持高动态范围
- 可以用于 IBL（基于图像的光照）

---

## 立方体贴图 vs 其他技术

### 立方体贴图 vs 2D 纹理

| 特性 | 立方体贴图 | 2D 纹理 |
|------|-----------|---------|
| **维度** | 3D 方向 | 2D 坐标 |
| **采样** | 方向向量 | UV 坐标 |
| **应用** | 环境映射、天空盒 | 普通纹理 |
| **内存** | 6 倍（6 个面） | 1 倍 |

### 立方体贴图 vs 球面贴图

| 特性 | 立方体贴图 | 球面贴图 |
|------|-----------|---------|
| **形状** | 立方体 | 球面 |
| **采样** | 方向向量 | 球面坐标 |
| **质量** | 均匀 | 两极有扭曲 |
| **性能** | 高 | 中等 |

---

## 实际应用

### 游戏开发

1. **天空盒**：
   - 渲染天空背景
   - 提供环境感
   - 增强沉浸感

2. **环境映射**：
   - 镜面反射
   - 水面反射
   - 金属表面反射

3. **IBL（基于图像的光照）**：
   - 环境光照
   - 间接光照
   - 真实感渲染

### 可视化应用

1. **环境展示**：
   - 360 度全景图
   - 虚拟环境
   - 场景预览

2. **反射效果**：
   - 产品展示
   - 建筑设计
   - 可视化渲染

---

## 总结

### Lesson 17 的核心价值

1. **理解立方体贴图**：学会了立方体贴图的概念和作用
2. **掌握天空盒**：理解了天空盒的实现方法
3. **实现环境映射**：学会了使用立方体贴图实现反射效果
4. **分离渲染和效果**：掌握了反射和天空盒的渲染流程

### 关键知识点

1. ✅ **立方体贴图**：由 6 个 2D 纹理组成的立方体纹理
2. ✅ **方向向量采样**：使用 3D 方向向量从立方体贴图中采样
3. ✅ **天空盒**：巨大的立方体，用于渲染天空背景
4. ✅ **环境映射**：使用立方体贴图模拟环境反射
5. ✅ **反射向量**：使用 `reflect()` 函数计算反射方向
6. ✅ **深度函数**：天空盒使用 `GL_LEQUAL` 确保显示在最远处
7. ✅ **视图矩阵**：天空盒需要移除视图矩阵的平移部分

### 实现立方体贴图和天空盒的步骤

1. **加载立方体贴图**：加载 6 个面的纹理
2. **设置纹理参数**：使用 `GL_CLAMP_TO_EDGE` 避免接缝
3. **渲染反射物体**：使用反射向量从立方体贴图中采样
4. **渲染天空盒**：最后渲染，使用特殊的深度函数和视图矩阵

### 下一步学习方向

1. **高级立方体贴图技术**：
   - HDR 立方体贴图
   - 动态立方体贴图
   - 立方体贴图过滤

2. **高级环境映射**：
   - 折射效果
   - 菲涅尔效果
   - 粗糙度映射

3. **IBL（基于图像的光照）**：
   - 环境光照
   - 间接光照
   - PBR 渲染

---

## 代码结构总结

```
Lesson 17
├── lesson17_1.cpp          # 立方体贴图和天空盒实现
├── 6.2.cubemaps.vs        # 反射效果顶点着色器
├── 6.2.cubemaps.fs        # 反射效果片段着色器
├── 6.2.skybox.vs          # 天空盒顶点着色器
└── 6.2.skybox.fs          # 天空盒片段着色器
```

---

## 完整代码示例

### 加载立方体贴图

```cpp
static unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = (nrChannels == 1) ? GL_RED : 
                           (nrChannels == 3) ? GL_RGB : GL_RGBA;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, 
                        width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
```

### 渲染流程

```cpp
// 第一步：渲染反射物体
m_shader->use();
// ... 设置矩阵和纹理 ...
glDrawArrays(GL_TRIANGLES, 0, 36);

// 第二步：渲染天空盒
glDepthFunc(GL_LEQUAL);
m_skyboxShader->use();
view = glm::mat4(glm::mat3(m_camera.GetViewMatrix()));
// ... 设置矩阵和纹理 ...
glDrawArrays(GL_TRIANGLES, 0, 36);
glDepthFunc(GL_LESS);
```

---

这就是 Lesson 17 的完整总结！通过本课程，您已经掌握了立方体贴图和天空盒的核心概念，这是实现高级渲染效果的重要技术。
