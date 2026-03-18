# Lesson 18: 几何着色器总结

## 📚 目录
- [概述](#概述)
- [几何着色器基础](#几何着色器基础)
- [Lesson 18: 几何着色器（沿法线爆炸）](#lesson-18-几何着色器沿法线爆炸)
- [Lesson 18-2: 法线可视化](#lesson-18-2-法线可视化)
- [着色器管线详解](#着色器管线详解)
- [几何着色器详解](#几何着色器详解)
- [常见问题](#常见问题)
- [几何着色器 vs 其他技术](#几何着色器-vs-其他技术)
- [实际应用](#实际应用)
- [总结](#总结)

---

## 概述

Lesson 18 介绍了 **几何着色器（Geometry Shader）** 的概念和使用方法。几何着色器位于顶点着色器与片段着色器之间，可以对图元进行增删、变形或改变图元类型，实现爆炸、法线可视化、程序化几何等效果。

### 学习目标

- ✅ 理解几何着色器的概念和在管线中的位置
- ✅ 掌握几何着色器的输入/输出（图元类型、顶点数）
- ✅ 学会在项目中启用几何着色器（Shader 三参数构造）
- ✅ 实现“沿法线爆炸”的几何变形
- ✅ 实现法线可视化（将法线画成线段）

---

## 几何着色器基础

### 什么是几何着色器？

**几何着色器（Geometry Shader）** 是顶点着色器与片段着色器之间的一可选阶段：

```
顶点着色器 → [几何着色器] → 片段着色器
```

- **输入**：以**图元**为单位（点、线、三角形等），每次处理一个完整图元的所有顶点
- **输出**：通过 `EmitVertex()`、`EndPrimitive()` 主动发射新的顶点与图元
- **特点**：可以增删顶点、改变图元类型（如三角形 → 线段）、做基于图元的变形

### 为什么需要几何着色器？

常见用途包括：

1. **几何变形**：沿法线爆炸、轮廓、挤出不规则体等
2. **法线/切线可视化**：把法线、切线画成短线，便于调试
3. **程序化几何**：由少顶点生成更多几何（草地、粒子等）
4. **图元类型转换**：三角形→线框、点→四边形等

### 几何着色器的工作原理

1. 接收**一个图元**的所有顶点（如一个三角形的 3 个顶点）
2. 通过 `in` 数组（如 `vPos[0..2]`）访问每个顶点的插值/传递数据
3. 使用 `EmitVertex()` 输出新顶点，用 `EndPrimitive()` 结束当前输出图元
4. 可输出与输入**不同类型**的图元（如 `triangles` in → `line_strip` out）

---

## Lesson 18: 几何着色器（沿法线爆炸）

### 功能特点

- 使用**背包模型**（backpack.obj）作为几何来源
- 着色器管线：**顶点 → 几何 → 片段**，几何着色器参与每一帧
- 效果：三角形在**法线方向**上做周期性位移，形成“爆炸—收回”的动画
- 通过 uniform `time` 驱动，便于观察几何着色器对顶点位置的修改

### 着色器结构

| 着色器 | 文件 | 作用 |
|--------|------|------|
| 顶点 | `9.2.geometry_shader.vs` | 传递位置、法线、UV，并做 MVP |
| 几何 | `9.2.geometry_shader.gs` | 输入三角形，沿法线偏移后仍输出三角形 |
| 片段 | `9.2.geometry_shader.fs` | 使用 `texture_diffuse1` 采样输出颜色 |

### 几何着色器核心逻辑（9.2.geometry_shader.gs）

```glsl
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vPos[];
in vec3 vNormal[];
in vec2 vTexCoord[];
out vec2 TexCoord;

uniform mat4 model, view, projection;
uniform float time;

void main()
{
    float magnitude = 0.2 * sin(time);
    for (int i = 0; i < 3; ++i)
    {
        vec3 offset = vPos[i] + vNormal[i] * magnitude;
        gl_Position = projection * view * model * vec4(offset, 1.0);
        TexCoord = vTexCoord[i];
        EmitVertex();
    }
    EndPrimitive();
}
```

- **输入图元**：`triangles`（每次 3 个顶点）
- **输出图元**：`triangle_strip`，最多 3 个顶点，即仍输出一个三角形
- **变形**：每个顶点沿自身法线偏移 `0.2 * sin(time)`，实现周期性“爆炸/收回”

### 关键代码（C++）

- 使用带几何着色器的 Shader 构造：`Shader(vsPath, fsPath, gsPath)`
- 每帧传入时间：`m_shader->setFloat("time", GetTime());`
- 模型与矩阵设置与 Lesson 12 类似，再调用 `m_model->Draw(*m_shader)` 即可

### 控制说明

- **WASD**：移动相机  
- **鼠标**：旋转视角  
- **滚轮**：缩放  
- **ESC**：退出  

---

## Lesson 18-2: 法线可视化

### 功能特点

- **两遍绘制**：先用默认着色器画带贴图的模型，再用法线可视化着色器在同一模型上画出法线线段
- **法线线段**：从每个顶点出发，沿法线方向画固定长度（如 0.1）的短线，便于检查法线方向与模型数据

### 着色器结构

| 着色器 | 文件 | 作用 |
|--------|------|------|
| 默认 | `9.3.default.vs` / `9.3.default.fs` | 常规模型渲染（MVP + diffuse 纹理） |
| 法线用顶点 | `9.3.normal_visualization.vs` | 输出世界空间位置、法线 |
| 法线用几何 | `9.3.normal_visualization.gs` | 三角形 → 多条线段（每顶点一条） |
| 法线用片段 | `9.3.normal_visualization.fs` | 用 uniform 颜色（如绿色）画线 |

### 几何着色器核心逻辑（9.3.normal_visualization.gs）

```glsl
layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in vec3 vPos[];
in vec3 vNormal[];

uniform mat4 view, projection;
const float normalLength = 0.1;

void main()
{
    for (int i = 0; i < 3; ++i)
    {
        vec3 start = vPos[i];
        vec3 end = vPos[i] + normalize(vNormal[i]) * normalLength;

        gl_Position = projection * view * vec4(start, 1.0);
        EmitVertex();
        gl_Position = projection * view * vec4(end, 1.0);
        EmitVertex();
        EndPrimitive();
    }
}
```

- **输入图元**：`triangles`（每个三角形 3 个顶点）
- **输出图元**：`line_strip`，每个三角形生成 **3 条线**（每个顶点一条）
- 每条线 2 个顶点：起点为 `vPos[i]`，终点为 `vPos[i] + normalize(vNormal[i]) * normalLength`
- `vPos`、`vNormal` 在顶点着色器中已转为世界空间，几何着色器只做 `view * projection` 即可

### 顶点着色器中的世界空间（9.3.normal_visualization.vs）

```glsl
vPos = (model * vec4(aPos, 1.0)).xyz;
vNormal = mat3(transpose(inverse(model))) * aNormal;
```

- 法线使用 `transpose(inverse(model))` 的 3×3 部分，保证在非等比缩放下方向正确。

### 渲染顺序（lesson18_2.cpp）

1. 使用 `m_defaultShader` 绘制模型（带纹理）
2. 使用 `m_normalShader` 再绘一遍同一模型，并设置 `normalColor`（如绿色），只画出法线线段

### 控制说明

与 Lesson 18 相同（WASD、鼠标、滚轮、ESC）。

---

## 着色器管线详解

### 完整管线（带几何着色器）

```
顶点着色器（每个顶点执行）
    ↓ 输出 per-vertex 数据（如 vPos, vNormal）
几何着色器（每个图元执行）
    ↓ 按图元读取顶点数组，发射新顶点/新图元
片段着色器（每个片段执行）
    ↓ 输出最终颜色
```

### 几何着色器的输入/输出声明

- **输入图元类型**（必选其一）：
  - `points`、`lines`、`lines_adjacency`、`triangles`、`triangles_adjacency`
- **输出图元类型**（必选其一）：
  - `points`、`line_strip`、`triangle_strip`
- **输出最大顶点数**：`max_vertices = N`，必须显式指定

常见组合示例：

| 输入 | 输出 | 用途示例 |
|------|------|----------|
| `triangles` | `triangle_strip`, max_vertices=3 | 三角形变形（Lesson 18 爆炸） |
| `triangles` | `line_strip`, max_vertices=2 | 每个顶点画一条线（Lesson 18-2 法线） |
| `points` | `triangle_strip`, max_vertices=4 | 点 → 四边形（如粒子、精灵） |

### EmitVertex 与 EndPrimitive

- **EmitVertex()**：把当前写入的 `gl_Position` 以及所有 `out` 变量压成一个顶点，写入当前输出图元
- **EndPrimitive()**：结束当前输出的图元，下一个 `EmitVertex()` 起开始新图元

例如法线可视化里，每轮循环 emit 两个顶点后 `EndPrimitive()`，就得到一条线段。

---

## 几何着色器详解

### 输入数组与图元顶点

几何着色器里，顶点着色器传来的 `out` 会变成**数组**：

```glsl
in vec3 vPos[];      // 一个图元内所有顶点的 vPos
in vec3 vNormal[];   // 一个图元内所有顶点的 vNormal
```

- 对 `triangles`，`vPos[0]、vPos[1]、vPos[2]` 对应该三角形的三个顶点
- 访问时需用常量索引（如循环中的 `i`），不可用非编译期常量

### 内置输入

- **gl_in[]**：每个元素对应输入图元的一个顶点，常用：
  - `gl_in[i].gl_Position`：该顶点在裁剪空间的位置（若顶点着色器写了 `gl_Position`）

本课程中几何着色器自行用 `model/view/projection` 和顶点数据算 `gl_Position`，较少直接依赖 `gl_in`。

### 法线变换（世界空间）

在 9.3 的顶点着色器里，法线用模型矩阵的“逆转置”变换到世界空间：

```glsl
vNormal = mat3(transpose(inverse(model))) * aNormal;
```

这样在存在非等比缩放时，法线仍与表面垂直，用于世界空间中的方向计算和线段绘制。

---

## 常见问题

### Q1: 几何着色器不生效或报错？

- 确认 Shader 使用了**三参数**构造并传入几何着色器路径：`Shader(vs, fs, gs)`。
- 确认几何着色器已正确编译，在 Shader 类中会打出 `"GEOMETRY"` 的编译错误信息。
- 确认 `layout (xxx) in;`、`layout (yyy, max_vertices = N) out;` 与当前绘制图元类型一致（例如用 `GL_TRIANGLES` 绘制时，几何着色器应为 `triangles` in）。

### Q2: 爆炸或法线方向不对？

- 爆炸（Lesson 18）：确认传入的是**模型空间**的 `vPos`、`vNormal`，且几何着色器里用 `model * vec4(offset, 1.0)` 再乘 `view * projection`；若顶点着色器里已乘过 model，几何里就不要再乘 model。
- 法线（Lesson 18-2）：法线必须用 `transpose(inverse(model))` 的 3×3 变换，且 `vPos` 为世界空间，线段长度用 `normalLength` 调节。

### Q3: 法线线段太短/太长或看不清？

- 在 `9.3.normal_visualization.gs` 中修改 `normalLength`（如 0.05～0.2）。
- 可调节 `normalColor`（如 `(1,1,0)` 黄色）以提高在不同背景上的可见度。

### Q4: 能否不要几何着色器，只用法线可视化？

- 可以。本课用法线几何着色器是为了“每个三角形顶点 → 一条线”的写法更清晰；也可以换成在 CPU 里生成线段的顶点、用普通顶点+片段着色器画线，只是数据量和代码结构会不同。

### Q5: 几何着色器对性能的影响？

- 几何着色器以**图元**为粒度执行，且可能显著增加或减少顶点数，对复杂网格和大量三角形影响较大。
- 适合用于调试、编辑器工具、少量特效；大规模场景要结合剔除、LOD 等谨慎使用。

---

## 几何着色器 vs 其他技术

| 特性 | 几何着色器 | 顶点着色器变形 | 计算着色器/CPU 生成几何 |
|------|------------|----------------|--------------------------|
| 图元级操作 | 支持（三角形/线等） | 仅顶点级 | 任意 |
| 改变图元类型 | 支持 | 不支持 | 支持 |
| 增删顶点 | 支持 | 不支持 | 支持 |
| 管线位置 | 固定（VS→GS→FS） | 仅 VS | 需额外 Pass 或管线设计 |

---

## 实际应用

1. **调试与工具**：法线/切线可视化、线框、网格预览（Lesson 18-2 即属此类）
2. **程序化几何**：粒子变四边形、草叶、毛发等由少顶点生成多三角形
3. **轮廓与描边**：沿法线挤出轮廓、背面膨胀等
4. **LOD / 删减**：在几何着色器里丢弃或简化图元（需注意驱动与扩展支持）

---

## 总结

### Lesson 18 核心要点

1. **几何着色器**：位于顶点与片段之间，以图元为单位输入，通过 `EmitVertex` / `EndPrimitive` 输出新图元。
2. **输入/输出声明**：`layout (triangles) in;`、`layout (triangle_strip, max_vertices = 3) out;` 等必须与绘制图元类型和需求一致。
3. **沿法线爆炸**：在几何着色器中对每个顶点做 `offset = vPos + vNormal * magnitude`，再用 MVP 写 `gl_Position`，`magnitude` 可用 `sin(time)` 做动画。
4. **Shader 用法**：本项目通过 `Shader(vsPath, fsPath, gsPath)` 启用几何着色器，并每帧传入 `time` 等 uniform。

### Lesson 18-2 核心要点

1. **法线可视化**：几何着色器输入三角形，对每个顶点发射一条 `line_strip`（2 个顶点：起点、起点+法线×长度）。
2. **世界空间**：顶点着色器输出世界空间 `vPos` 与 `vNormal`，几何着色器只用 `view * projection` 得到裁剪空间位置。
3. **两遍绘制**：先默认着色器画模型，再用法线着色器叠加上色线段，便于调试。

### 代码与文件结构

```
Lesson 18
├── lesson18_1.cpp              # 几何着色器（沿法线爆炸）
├── lesson18_2.cpp               # 法线可视化
├── 9.2.geometry_shader.vs/fs/gs # 爆炸效果管线
├── 9.3.default.vs/fs            # 默认模型渲染
└── 9.3.normal_visualization.vs/fs/gs  # 法线可视化管线
```

---

通过 Lesson 18 与 18-2，可以掌握几何着色器的基本写法、图元类型变换以及与现有模型渲染管线的结合方式，为后续轮廓、程序化几何等效果打基础。
