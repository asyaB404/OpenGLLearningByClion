# Lesson 15: 混合透明纹理总结

## 📚 目录
- [概述](#概述)
- [混合基础](#混合基础)
- [Lesson 15: 透明纹理渲染](#lesson-15-透明纹理渲染)
- [混合详解](#混合详解)
- [透明物体排序](#透明物体排序)
- [渲染流程详解](#渲染流程详解)
- [着色器详解](#着色器详解)
- [纹理处理](#纹理处理)
- [常见问题](#常见问题)
- [混合 vs 其他技术](#混合-vs-其他技术)
- [实际应用](#实际应用)
- [总结](#总结)

---

## 概述

Lesson 15 介绍了 **混合（Blending）** 和 **透明纹理（Transparent Textures）** 的概念和使用方法。通过混合技术，我们可以渲染透明或半透明的物体，实现玻璃、窗户、水面等效果。

### 学习目标

- ✅ 理解混合的概念和作用
- ✅ 掌握混合函数的工作原理
- ✅ 学会渲染透明纹理
- ✅ 理解透明物体的排序问题
- ✅ 掌握从远到近渲染透明物体的方法

---

## 混合基础

### 什么是混合？

**混合（Blending）** 是 OpenGL 用来混合新片段颜色和已有颜色缓冲中颜色的机制。它允许我们实现透明效果，让后面的物体能够透过前面的物体显示出来。

### 为什么需要混合？

在 3D 场景中，有些物体是透明的或半透明的：

- **透明物体**：玻璃、窗户、水面
- **半透明物体**：烟雾、火焰、粒子效果
- **UI 元素**：半透明面板、对话框

**没有混合的问题**：
- 透明物体完全不透明（不真实）
- 无法看到后面的物体
- 无法实现半透明效果

**有混合的好处**：
- 正确显示透明物体
- 可以看到后面的物体
- 实现真实的透明效果

### 混合的工作原理

混合通过以下公式计算最终颜色：

```
最终颜色 = 源颜色 × 源因子 + 目标颜色 × 目标因子
```

**术语说明**：
- **源颜色（Source Color）**：新片段的颜色（正在绘制的物体）
- **目标颜色（Destination Color）**：颜色缓冲中已有的颜色（已经绘制的物体）
- **源因子（Source Factor）**：源颜色的混合因子
- **目标因子（Destination Factor）**：目标颜色的混合因子

### 混合公式详解

OpenGL 提供了多种混合模式，最常用的是 **Alpha 混合**：

```glsl
最终颜色 = 源颜色 × 源Alpha + 目标颜色 × (1 - 源Alpha)
```

**示例**：
- 源颜色：RGBA(1.0, 0.0, 0.0, 0.5) - 红色，50% 不透明
- 目标颜色：RGBA(0.0, 0.0, 1.0, 1.0) - 蓝色，100% 不透明
- 最终颜色 = (1.0, 0.0, 0.0) × 0.5 + (0.0, 0.0, 1.0) × 0.5 = (0.5, 0.0, 0.5) - 紫色

---

## Lesson 15: 透明纹理渲染

### 功能特点

- **两个立方体**：场景中有两个不透明的立方体
- **地板**：场景中有地板作为背景
- **透明窗户**：场景中有 5 个透明窗户（使用透明纹理）
- **相机系统**：可以自由移动相机观察效果
- **正确排序**：透明窗户从远到近渲染，确保正确混合

### 场景设置

```cpp
// 透明窗户位置
std::vector<glm::vec3> m_windows = {
    glm::vec3(-1.5f, 0.0f, -0.48f),
    glm::vec3( 1.5f, 0.0f,  0.51f),
    glm::vec3( 0.0f, 0.0f,  0.7f),
    glm::vec3(-0.3f, 0.0f, -2.3f),
    glm::vec3( 0.5f, 0.0f, -0.6f)
};
```

### 关键代码

#### 启用混合

```cpp
// 启用混合
glEnable(GL_BLEND);

// 设置混合函数：源颜色 * 源Alpha + 目标颜色 * (1 - 源Alpha)
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

**参数说明**：
- `GL_SRC_ALPHA`：源因子 = 源颜色的 Alpha 值
- `GL_ONE_MINUS_SRC_ALPHA`：目标因子 = 1 - 源颜色的 Alpha 值

#### 渲染顺序

```cpp
// 1. 先渲染不透明物体（立方体和地面）
glDrawArrays(...);  // 立方体
glDrawArrays(...);  // 地面

// 2. 按距离排序透明物体（从远到近）
std::map<float, glm::vec3> sorted;
for (unsigned int i = 0; i < m_windows.size(); i++)
{
    float distance = glm::length(m_camera.Position - m_windows[i]);
    sorted[distance] = m_windows[i];
}

// 3. 从远到近渲染透明物体
for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); 
     it != sorted.rend(); ++it)
{
    // 绘制透明窗户
    glDrawArrays(...);
}
```

### 视觉效果

- **立方体**：正常渲染，不透明
- **地板**：正常渲染，不透明
- **透明窗户**：半透明，可以看到后面的物体
- **正确混合**：透明窗户从远到近渲染，混合效果正确

### 控制说明

- **WASD**：移动相机
- **鼠标移动**：旋转视角
- **滚轮**：缩放视野
- **ESC**：退出程序

---

## 混合详解

### 混合函数

OpenGL 提供了多种混合因子：

| 混合因子 | 说明 | 值 |
|---------|------|-----|
| `GL_ZERO` | 0 | (0, 0, 0, 0) |
| `GL_ONE` | 1 | (1, 1, 1, 1) |
| `GL_SRC_COLOR` | 源颜色 | (Rs, Gs, Bs, As) |
| `GL_ONE_MINUS_SRC_COLOR` | 1 - 源颜色 | (1-Rs, 1-Gs, 1-Bs, 1-As) |
| `GL_DST_COLOR` | 目标颜色 | (Rd, Gd, Bd, Ad) |
| `GL_ONE_MINUS_DST_COLOR` | 1 - 目标颜色 | (1-Rd, 1-Gd, 1-Bd, 1-Ad) |
| `GL_SRC_ALPHA` | 源 Alpha | (As, As, As, As) |
| `GL_ONE_MINUS_SRC_ALPHA` | 1 - 源 Alpha | (1-As, 1-As, 1-As, 1-As) |
| `GL_DST_ALPHA` | 目标 Alpha | (Ad, Ad, Ad, Ad) |
| `GL_ONE_MINUS_DST_ALPHA` | 1 - 目标 Alpha | (1-Ad, 1-Ad, 1-Ad, 1-Ad) |

**设置混合函数**：
```cpp
glBlendFunc(srcFactor, dstFactor);
```

**常用组合**：

| 组合 | 效果 | 使用场景 |
|------|------|----------|
| `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA` | 标准 Alpha 混合 | 透明物体（最常用） |
| `GL_ONE, GL_ZERO` | 不混合（默认） | 不透明物体 |
| `GL_ONE, GL_ONE` | 加法混合 | 发光效果、粒子 |
| `GL_SRC_ALPHA, GL_ONE` | 加法混合（带 Alpha） | 发光粒子 |
| `GL_DST_COLOR, GL_ZERO` | 乘法混合 | 暗化效果 |

### 混合方程

OpenGL 还提供了更灵活的混合方程：

```cpp
glBlendEquation(mode);
```

**混合模式**：

| 模式 | 公式 | 说明 |
|------|------|------|
| `GL_FUNC_ADD` | 源 × 源因子 + 目标 × 目标因子 | 标准混合（默认） |
| `GL_FUNC_SUBTRACT` | 源 × 源因子 - 目标 × 目标因子 | 减法混合 |
| `GL_FUNC_REVERSE_SUBTRACT` | 目标 × 目标因子 - 源 × 源因子 | 反向减法混合 |
| `GL_MIN` | min(源, 目标) | 取最小值 |
| `GL_MAX` | max(源, 目标) | 取最大值 |

**完整设置**：
```cpp
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glBlendEquation(GL_FUNC_ADD);
```

### 混合与深度测试

**重要**：混合和深度测试的关系：

1. **深度测试在混合之前**：只有通过深度测试的片段才会进行混合
2. **透明物体需要特殊处理**：
   - 禁用深度写入：`glDepthMask(GL_FALSE)`
   - 从远到近渲染：确保正确混合

**为什么需要禁用深度写入？**

如果不禁用深度写入，透明物体会写入深度缓冲，导致后面的透明物体被丢弃（深度测试失败）。

**解决方案**：
```cpp
// 渲染透明物体时
glDepthMask(GL_FALSE);  // 禁用深度写入
// ... 渲染透明物体 ...
glDepthMask(GL_TRUE);   // 恢复深度写入
```

---

## 透明物体排序

### 为什么需要排序？

透明物体必须从远到近渲染，才能正确混合。如果顺序错误，会导致：

- **错误的混合**：近处的物体会错误地混合到远处的物体
- **视觉错误**：透明效果不正确
- **深度问题**：深度测试可能失败

### 排序方法

#### 方法 1：按距离排序（本课程使用）

```cpp
// 计算每个透明物体到相机的距离
std::map<float, glm::vec3> sorted;
for (unsigned int i = 0; i < m_windows.size(); i++)
{
    float distance = glm::length(m_camera.Position - m_windows[i]);
    sorted[distance] = m_windows[i];
}

// 从远到近渲染（使用反向迭代器）
for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); 
     it != sorted.rend(); ++it)
{
    // 绘制透明物体
}
```

**优点**：
- 简单直接
- 适用于大多数情况

**缺点**：
- 需要每帧重新排序（性能开销）
- 对于大量透明物体，性能可能成为问题

#### 方法 2：预排序

```cpp
// 在初始化时排序（如果物体位置固定）
std::sort(m_windows.begin(), m_windows.end(), 
    [&camera](const glm::vec3& a, const glm::vec3& b) {
        return glm::length(camera.Position - a) > 
               glm::length(camera.Position - b);
    });
```

**优点**：
- 不需要每帧排序
- 性能更好

**缺点**：
- 只适用于静态场景
- 相机移动时需要重新排序

#### 方法 3：使用深度排序算法

对于复杂的透明物体（如多个三角形），可以使用更复杂的排序算法：

- **BSP 树**：二叉空间分割树
- **画家算法**：从远到近绘制
- **OIT（Order Independent Transparency）**：顺序无关透明

### 排序的局限性

**问题**：当透明物体相交时，简单的距离排序无法正确处理。

**示例**：
```
相机视角：
  [透明物体 A]  [透明物体 B]
      \            /
       \          /
        \        /
         \      /
          \    /
           \  /
            \/
```

**解决方案**：
1. **分离透明物体**：避免透明物体相交
2. **使用 OIT 技术**：顺序无关透明（高级技术）
3. **接受不完美**：对于大多数情况，简单的距离排序已经足够

---

## 渲染流程详解

### 完整的渲染流程

#### 第一步：清除缓冲

```cpp
glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

#### 第二步：渲染不透明物体

```cpp
// 启用深度测试和深度写入
glEnable(GL_DEPTH_TEST);
glDepthMask(GL_TRUE);

// 禁用混合（不透明物体不需要混合）
glDisable(GL_BLEND);

// 渲染立方体
glDrawArrays(GL_TRIANGLES, 0, 36);  // 第一个立方体
glDrawArrays(GL_TRIANGLES, 0, 36);  // 第二个立方体

// 渲染地板
glDrawArrays(GL_TRIANGLES, 0, 6);
```

**结果**：
- 不透明物体被正确渲染
- 深度缓冲被更新

#### 第三步：渲染透明物体

```cpp
// 启用混合
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// 禁用深度写入（但保持深度测试）
glDepthMask(GL_FALSE);

// 按距离排序透明物体
std::map<float, glm::vec3> sorted;
// ... 排序代码 ...

// 从远到近渲染透明物体
for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); 
     it != sorted.rend(); ++it)
{
    // 绘制透明窗户
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// 恢复深度写入
glDepthMask(GL_TRUE);
```

**结果**：
- 透明物体被正确混合
- 可以看到后面的物体

### 渲染顺序的重要性

**错误的渲染顺序**：
```
1. 渲染透明物体 A（近）
2. 渲染透明物体 B（远）
```

**结果**：物体 B 无法正确混合到物体 A，因为物体 A 已经写入颜色缓冲。

**正确的渲染顺序**：
```
1. 渲染透明物体 B（远）
2. 渲染透明物体 A（近）
```

**结果**：物体 A 正确混合到物体 B，效果正确。

---

## 着色器详解

### 顶点着色器 (`3.2.blending.vs`)

```glsl
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

**功能**：
- 接收顶点位置和纹理坐标
- 应用模型、视图和投影变换
- 传递纹理坐标到片段着色器

### 片段着色器 (`3.2.blending.fs`)

```glsl
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoords);
}
```

**功能**：
- 从纹理采样颜色（包括 Alpha 通道）
- 输出最终片段颜色（RGBA）

**关键**：纹理必须包含 Alpha 通道（RGBA 格式），Alpha 值用于混合计算。

---

## 纹理处理

### 透明纹理的要求

透明纹理必须满足以下要求：

1. **Alpha 通道**：纹理必须是 RGBA 格式（4 通道）
2. **Alpha 值**：
   - `1.0`：完全不透明
   - `0.0`：完全透明
   - `0.0 < Alpha < 1.0`：半透明

### 纹理加载

```cpp
static unsigned int loadTexture(const char* path, bool flipVertically = true)
{
    // ... 加载纹理 ...
    
    // 根据通道数确定格式
    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;  // 透明纹理使用 RGBA
    
    // ... 设置纹理参数 ...
    
    // 对于透明纹理（RGBA），使用 GL_CLAMP_TO_EDGE 防止边缘半透明
    // 对于不透明纹理，使用 GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 
                    format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 
                    format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
}
```

### 纹理包装模式

**为什么透明纹理使用 `GL_CLAMP_TO_EDGE`？**

透明纹理的边缘通常有半透明的像素（用于抗锯齿）。如果使用 `GL_REPEAT`，边缘的半透明像素会重复，导致不自然的边缘效果。

**对比**：
- `GL_REPEAT`：适合不透明纹理（如砖墙、地板）
- `GL_CLAMP_TO_EDGE`：适合透明纹理（如窗户、玻璃）

### Alpha 测试（可选优化）

对于完全透明（Alpha = 0）的像素，可以使用 Alpha 测试提前丢弃：

```glsl
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    vec4 texColor = texture(texture1, TexCoords);
    
    // Alpha 测试：完全透明的像素直接丢弃
    if (texColor.a < 0.1)
        discard;
    
    FragColor = texColor;
}
```

**优点**：
- 减少不必要的混合计算
- 提高性能

**缺点**：
- 需要额外的条件判断
- 可能影响边缘的平滑过渡

---

## 常见问题

### Q1: 为什么透明物体看起来不正确？

**A:** 可能的原因：

1. **渲染顺序错误**：透明物体没有从远到近渲染
2. **深度写入未禁用**：透明物体写入了深度缓冲
3. **混合函数错误**：使用了错误的混合函数
4. **纹理 Alpha 通道问题**：纹理没有 Alpha 通道或 Alpha 值不正确

**解决方法**：
```cpp
// 1. 禁用深度写入
glDepthMask(GL_FALSE);

// 2. 从远到近渲染
// ... 排序代码 ...

// 3. 使用正确的混合函数
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

### Q2: 透明物体被不透明物体遮挡怎么办？

**A:** 这是正常的。透明物体仍然需要深度测试，只有通过深度测试的片段才会被绘制。

**如果希望透明物体显示在前面**：
- 调整渲染顺序：先渲染不透明物体，再渲染透明物体
- 或者调整透明物体的位置

### Q3: 如何实现完全透明的效果？

**A:** 在片段着色器中丢弃 Alpha 为 0 的像素：

```glsl
void main()
{
    vec4 texColor = texture(texture1, TexCoords);
    if (texColor.a < 0.01)
        discard;
    FragColor = texColor;
}
```

### Q4: 混合的性能影响？

**A:** 混合的性能开销：

- **读取开销**：需要读取颜色缓冲（中等开销）
- **计算开销**：混合计算（很小）
- **写入开销**：写入混合后的颜色（正常）

**优化建议**：
1. 尽量减少透明物体的数量
2. 使用 Alpha 测试提前丢弃完全透明的像素
3. 对于静态场景，预排序透明物体

### Q5: 如何处理多个透明物体相交？

**A:** 简单的距离排序无法正确处理相交的透明物体。

**解决方案**：
1. **分离透明物体**：避免透明物体相交
2. **使用 OIT 技术**：顺序无关透明（高级技术）
3. **接受不完美**：对于大多数情况，简单的距离排序已经足够

### Q6: 如何实现发光效果？

**A:** 使用加法混合：

```cpp
glBlendFunc(GL_ONE, GL_ONE);  // 加法混合
glBlendEquation(GL_FUNC_ADD);
```

**效果**：颜色会叠加，产生发光效果。

### Q7: 如何实现暗化效果？

**A:** 使用乘法混合：

```cpp
glBlendFunc(GL_DST_COLOR, GL_ZERO);  // 乘法混合
```

**效果**：新颜色会与已有颜色相乘，产生暗化效果。

### Q8: 透明物体的深度测试问题？

**A:** 透明物体需要特殊处理：

```cpp
// 渲染透明物体时
glDepthMask(GL_FALSE);  // 禁用深度写入
glEnable(GL_DEPTH_TEST);  // 但保持深度测试

// ... 渲染透明物体 ...

glDepthMask(GL_TRUE);  // 恢复深度写入
```

**原因**：
- 深度测试：确保透明物体不会显示在不透明物体前面
- 禁用深度写入：确保后面的透明物体不会被丢弃

---

## 混合 vs 其他技术

### 混合 vs 深度测试

| 特性 | 混合 | 深度测试 |
|------|------|---------|
| **用途** | 混合颜色 | 确定前后关系 |
| **结果** | 混合后的颜色 | 丢弃或保留片段 |
| **应用** | 透明物体 | 所有物体 |
| **性能** | 中等开销 | 很小开销 |

### 混合 vs 模板测试

| 特性 | 混合 | 模板测试 |
|------|------|---------|
| **用途** | 混合颜色 | 标记区域 |
| **结果** | 混合后的颜色 | 丢弃或保留片段 |
| **应用** | 透明物体 | 轮廓、遮罩 |
| **性能** | 中等开销 | 很小开销 |

### 混合 vs 多重采样（MSAA）

| 特性 | 混合 | 多重采样 |
|------|------|---------|
| **用途** | 透明效果 | 抗锯齿 |
| **结果** | 混合颜色 | 平滑边缘 |
| **应用** | 透明物体 | 所有物体 |
| **性能** | 中等开销 | 高开销 |

---

## 实际应用

### 游戏开发

1. **透明物体**：
   - 玻璃窗户
   - 水面
   - 透明材质

2. **特效**：
   - 烟雾
   - 火焰
   - 粒子效果

3. **UI 元素**：
   - 半透明面板
   - 对话框
   - HUD 元素

### UI 开发

1. **半透明面板**：
   - 菜单背景
   - 对话框背景
   - 提示框

2. **按钮效果**：
   - 悬停效果
   - 按下效果
   - 禁用状态

### 可视化应用

1. **数据可视化**：
   - 半透明图表
   - 数据叠加
   - 信息面板

2. **3D 可视化**：
   - 透明模型
   - 半透明辅助线
   - 透明标注

---

## 总结

### Lesson 15 的核心价值

1. **理解混合**：学会了混合的概念和作用
2. **掌握混合函数**：理解了混合函数的工作原理
3. **实现透明效果**：学会了渲染透明纹理
4. **解决排序问题**：掌握了透明物体的排序方法

### 关键知识点

1. ✅ **混合**：混合新片段颜色和已有颜色的机制
2. ✅ **混合公式**：`最终颜色 = 源颜色 × 源因子 + 目标颜色 × 目标因子`
3. ✅ **Alpha 混合**：`源颜色 × Alpha + 目标颜色 × (1 - Alpha)`
4. ✅ **混合函数**：`glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`
5. ✅ **透明物体排序**：必须从远到近渲染
6. ✅ **深度写入**：透明物体需要禁用深度写入
7. ✅ **纹理 Alpha 通道**：透明纹理必须是 RGBA 格式

### 实现透明效果的步骤

1. **启用混合**：`glEnable(GL_BLEND)`
2. **设置混合函数**：`glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)`
3. **禁用深度写入**：`glDepthMask(GL_FALSE)`
4. **按距离排序**：从远到近渲染透明物体
5. **使用 RGBA 纹理**：纹理必须包含 Alpha 通道

### 下一步学习方向

1. **高级混合技术**：
   - 多重混合模式
   - 自定义混合方程
   - 混合优化

2. **顺序无关透明（OIT）**：
   - 加权混合（Weighted Blended）
   - 深度剥离（Depth Peeling）
   - 自适应透明度（Adaptive Transparency）

3. **实际应用**：
   - 游戏中的透明效果
   - UI 中的半透明元素
   - 可视化中的透明叠加

---

## 代码结构总结

```
Lesson 15
├── lesson15_1.cpp          # 混合透明纹理实现
├── 3.2.blending.vs        # 混合顶点着色器
└── 3.2.blending.fs        # 混合片段着色器
```

---

## 完整代码示例

### 启用混合

```cpp
// 启用混合
glEnable(GL_BLEND);

// 设置混合函数：标准 Alpha 混合
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// 设置混合方程：加法混合（默认）
glBlendEquation(GL_FUNC_ADD);
```

### 渲染透明物体

```cpp
// 禁用深度写入（但保持深度测试）
glDepthMask(GL_FALSE);

// 按距离排序透明物体
std::map<float, glm::vec3> sorted;
for (unsigned int i = 0; i < m_windows.size(); i++)
{
    float distance = glm::length(m_camera.Position - m_windows[i]);
    sorted[distance] = m_windows[i];
}

// 从远到近渲染透明物体
for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); 
     it != sorted.rend(); ++it)
{
    // 绘制透明物体
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// 恢复深度写入
glDepthMask(GL_TRUE);
```

### 纹理加载（透明纹理）

```cpp
// 加载 RGBA 纹理
unsigned int texture = loadTexture("window.png");

// 在 loadTexture 函数中：
// 对于 RGBA 纹理，使用 GL_CLAMP_TO_EDGE
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 
                format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 
                format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
```

---

这就是 Lesson 15 的完整总结！通过本课程，您已经掌握了混合和透明纹理的核心概念，这是实现真实感渲染的重要技术。
