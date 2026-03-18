# Lesson 19: 实例化渲染总结

## 📚 目录
- [概述](#概述)
- [实例化与普通绘制的区别](#实例化与普通绘制的区别)
- [实例化的好处与坏处](#实例化的好处与坏处)
- [Lesson 19: 实例化实现](#lesson-19-实例化实现)
- [实例数组与顶点属性详解](#实例数组与顶点属性详解)
- [着色器与绘制流程](#着色器与绘制流程)
- [常见问题](#常见问题)
- [总结](#总结)

---

## 概述

Lesson 19 介绍 **实例化渲染（Instancing）**：用**同一种网格**、**不同的每实例数据**（如模型矩阵），在**一次绘制调用**中画出大量物体（如成千上万个岩石、树木、子弹）。本课以“一个行星 + 大量小行星”为例，对比**普通多次绘制**与**单次实例化绘制**的差异，并说明实例化的适用场景与取舍。

### 学习目标

- ✅ 理解实例化的概念和 `glDrawElementsInstanced` 的作用
- ✅ 掌握实例数组（Instanced Array）与 `glVertexAttribDivisor`
- ✅ 明确**实例化与之前课程（普通绘制）的差别**
- ✅ 理解实例化的**好处与坏处**，知道何时使用、何时不用

---

## 实例化与普通绘制的区别

### 之前课程的做法：普通绘制（每物体一次调用）

在 Lesson 5、Lesson 12 等课程中，画多个物体时通常这样写：

```cpp
// 伪代码：画 N 个立方体/模型
for (int i = 0; i < N; i++)
{
    shader.setMat4("model", modelMatrices[i]);  // 每帧 CPU 设置 uniform
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);  // 每物体一次 Draw Call
}
```

特点：

- 每个物体一次 **Draw Call**（`glDrawElements` / `glDrawArrays`）
- 每个物体的 **model 矩阵**由 CPU 通过 **uniform** 每帧传入
- 顶点数据（位置、法线、UV 等）是**同一份**，只是通过改 uniform 在不同位置/姿态画出来

当 N 很大（例如几万、十万）时：

- **Draw Call 次数** = N，驱动与 GPU 要处理大量小批次，**CPU 负担高、批次切换开销大**
- **CPU–GPU 通信**：每帧 N 次 setMat4 + N 次 draw，容易成为瓶颈

### 实例化：一次调用画 N 个“实例”

实例化的思路是：

- **顶点数据**仍然只有一份（同一个 mesh）
- **每实例数据**（例如 4×4 模型矩阵）放在显存里，用**顶点属性**的形式传给着色器，并设置 **divisor = 1**，表示“每个实例取一次”
- **一次** `glDrawElementsInstanced` / `glDrawArraysInstanced` 就画出 N 个物体

对比（概念上）：

| 项目           | 普通绘制（之前课程）     | 实例化（Lesson 19）           |
|----------------|--------------------------|--------------------------------|
| Draw Call 次数 | N 个物体 = N 次          | N 个物体 = **1 次**（每 mesh） |
| 每物体 model   | CPU 每帧 setMat4 + 每物体一次 draw | 预先放在 VBO，GPU 按实例索引取 |
| 顶点属性       | 只有 0,1,2…（顶点共用）  | 再增加 7,8,9,10（mat4，**每实例**） |
| 适用场景       | 物体少、每个物体逻辑不同 | 物体多、同一 mesh、仅变换不同   |

因此，**和之前课程的差别**可以概括为：

1. **API**：从多次 `glDrawElements` 变成一次 `glDrawElementsInstanced(..., instanceCount)`。
2. **每实例数据来源**：从 CPU 每帧传 uniform，变成放在 **VBO 里**，用 **vertex attribute + divisor 1** 传入。
3. **性能目标**：在“大量相同网格、不同变换”的场景下，减少 Draw Call 和 CPU 开销，提高 GPU 利用率。

---

## 实例化的好处与坏处

### 好处

1. **大幅减少 Draw Call**
   - 例如 10 万个岩石：普通绘制要 10 万次 draw；实例化可以按 mesh 合并，每个 mesh 只 1 次 `glDrawElementsInstanced`。
   - 减少驱动与 GPU 的批次切换，降低 CPU 侧的开销。

2. **降低 CPU–GPU 通信**
   - 不再每帧对每个物体 `setMat4("model", ...)` 再 draw；矩阵一次性放在 VBO，GPU 自己按实例 ID 取用。
   - 适合“每实例数据仅变换、少量”的情况（如只有 model 矩阵）。

3. **更易达到高帧率**
   - 在草木、岩石、子弹、建筑重复件等“同 mesh、大量实例”的场景中，实例化几乎是标准做法。

4. **实现简单**
   - 只需：一个存矩阵的 VBO、在 VAO 上开 4 个 attribute（mat4 的 4 列）、`glVertexAttribDivisor(7~10, 1)`，顶点着色器里用 `in mat4 aInstanceMatrix` 替代 uniform model。

### 坏处与限制

1. **只适合“同一 mesh、不同变换”**
   - 若每个物体**几何不同**或**材质/纹理差异大**，无法用同一套实例数据一次画；要么拆成多批实例化，要么退回普通绘制。

2. **每实例数据要预先或按批准备好**
   - 矩阵等要写入 VBO；若每帧**大量物体位置/姿态变化**，需要每帧更新该 VBO（或双缓冲），有上传成本。
   - 对“完全静态”或“按批更新”的物体最友好。

3. **剔除与 LOD 更复杂**
   - 实例化一次提交一整批；若要做**视锥剔除**或**LOD**，需要在 CPU 端按可见性/距离筛选实例，只把“可见实例”的矩阵写入 VBO，或使用 Indirect Draw + 实例 ID 在着色器里做丢弃，逻辑比“每个物体一个 Draw Call”复杂。

4. **调试与灵活性**
   - 无法在“某一帧只画第 k 个物体”时像普通 draw 那样直观；需要从实例 ID 或实例数据反推。

5. **OpenGL 版本与扩展**
   - 本课用的 `glDrawElementsInstanced` 和 `glVertexAttribDivisor` 在 OpenGL 3.3 / ES 3.0 即可用，一般无问题；更高级的用法（如 Indirect）需要更高版本或扩展。

**何时用实例化**：同一种 mesh、数量大（几百到数十万）、每实例主要是变换（和少量参数）不同。  
**何时用普通绘制**：物体少、或每个物体 mesh/材质差异大、或需要逐物体逻辑简单清晰时。

---

## Lesson 19: 实例化实现

### 功能特点

- **场景**：中心 1 个行星（planet）+ 环形区域内大量小行星（rock），小行星使用**同一 rock 模型**。
- **行星**：用**普通绘制**（一次 `planet.Draw(shader)`），和之前课程一致。
- **小行星**：用**实例化**——所有小行星的模型矩阵预先算好、放进 VBO，每个 rock mesh 只调用一次 `glDrawElementsInstanced(..., instanceCount)`。
- **相机**：初始在 `(0, 0, 155)`，便于观察整圈小行星；WASD/鼠标/滚轮控制与其它课程一致。

### 资源路径

- 模型：`engine/assets/models/planet/planet.obj`、`engine/assets/models/rock/rock.obj`（与参考一致，放在 assets/models 下）。

### 核心流程简述

1. **生成 N 个模型矩阵**（例如 10000）：在环形上均匀分角，加随机位移、随机缩放、随机旋转，得到 `modelMatrices[i]`。
2. **实例 VBO**：`glBufferData(GL_ARRAY_BUFFER, N * sizeof(glm::mat4), modelMatrices.data(), ...)`，把矩阵上传到显存。
3. **绑定到 rock 的每个 mesh 的 VAO**：对每个 mesh，绑定其 VAO，再绑定实例 VBO，为 **location 7~10**（mat4 的 4 列）设置 `glVertexAttribPointer` 和 `glVertexAttribDivisor(7~10, 1)`。
4. **渲染**：  
   - 行星：`planetShader` + `planet.Draw(planetShader)`（普通绘制）。  
   - 小行星：`asteroidShader`，绑定 rock 的 diffuse 纹理，对每个 mesh：`glBindVertexArray(mesh.VAO); glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, N);`。

注意：本项目中 Mesh 已占用 location 0~6（位置、法线、UV、切线等），因此**实例矩阵使用 location 7~10**，与参考中“用 3、4、5、6”不同，避免覆盖现有顶点属性。

---

## 实例数组与顶点属性详解

### glVertexAttribDivisor(attribute, divisor)

- `divisor = 0`（默认）：该属性是**每顶点**的，每个顶点从 VBO 里按步长取一份。
- `divisor = 1`：该属性是**每实例**的，每个实例取一份，所有顶点在该实例内共用这份值。

因此，把 mat4 放在 VBO 里，每行一个实例的 4×4 矩阵，用 4 个 attribute（每列一个 vec4），并把这 4 个 attribute 的 divisor 都设为 1，GPU 就会在绘制第 i 个实例时，自动从第 i 个“矩阵行”里取数据，顶点着色器里就能用 `in mat4 aInstanceMatrix` 得到当前实例的模型矩阵。

### 为何用 location 7~10

- 本课使用的 Mesh 在 `setupMesh()` 里已经占用了 0~6（位置、法线、纹理坐标、切线、副切线、骨骼 ID、骨骼权重）。
- 实例矩阵是 mat4，在 GLSL 中占 4 个连续 location；因此使用 7、8、9、10，并在顶点着色器中声明 `layout(location = 7) in mat4 aInstanceMatrix;`，与 C++ 端为 7~10 的 `glVertexAttribPointer` 一致。

### 矩阵在 VBO 中的布局

- 每个实例一个 `glm::mat4`（列主序），连续存放。
- `glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);` 表示第 1 列从偏移 0 开始，步长为一整个 mat4。
- 同理，8、9、10 的偏移分别为 1×、2×、3× `sizeof(glm::vec4)`。

---

## 着色器与绘制流程

### 行星（普通绘制）

- **10.3.planet.vs**：与之前课程一致，`uniform mat4 model;`，`gl_Position = projection * view * model * vec4(aPos, 1.0);`
- **10.3.planet.fs**：采样 `texture_diffuse1` 输出颜色。
- 每帧设置一次 `model`（平移 + 缩放），调用 `planet.Draw(planetShader)`，即多次 `glDrawElements`（每个 mesh 一次）。

### 小行星（实例化）

- **10.3.asteroids.vs**：**无** uniform model，改为 `layout(location = 7) in mat4 aInstanceMatrix;`，使用 `gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);`
- **10.3.asteroids.fs**：与 planet 相同，采样 diffuse 纹理。
- 每帧只设置 `projection`、`view` 和纹理；对每个 rock mesh 调用一次 `glDrawElementsInstanced(..., instanceCount)`，由 GPU 按实例 ID 取 `aInstanceMatrix`。

---

## 常见问题

### Q: 实例化后还能对每个实例做不同纹理/材质吗？

可以，但需要额外数据：例如再开一个“每实例”的 attribute（如材质 ID 或纹理索引），在片段着色器里根据该 ID 选纹理或 uniform 数组；或使用纹理数组/图集。本课为简化，所有小行星共用同一 diffuse 纹理。

### Q: 为什么小行星用 location 7~10 而不是 3~6？

因为当前 Mesh 类已经用 3~6 存切线、副切线、骨骼等；若再占 3~6 会覆盖。使用 7~10 与现有顶点属性不冲突。

### Q: 实例数量很大时会不会爆显存？

主要占用是：`N * sizeof(glm::mat4)`。例如 N=100000 约 6.4MB，一般可接受。若 N 更大，可配合视锥剔除或 LOD，只上传可见实例的矩阵。

### Q: 和“用 uniform 数组传多个 model”有什么区别？

Uniform 数组有大小限制（通常几百到几千），且每帧要从 CPU 上传；实例化把数据放在 VBO，可支持数万、十万级实例，且无需每帧传大量 uniform。

---

## 总结

- **实例化** = 同一 mesh + 每实例数据（如模型矩阵）放在 VBO + `glVertexAttribDivisor(..., 1)` + `glDrawElementsInstanced(..., instanceCount)`，**一次调用画出多份几何**。
- **与之前课程的差别**：从“每物体一个 Draw Call + 每帧 setMat4”变成“每类 mesh 一个 Instanced Draw Call + 矩阵在显存里按实例索引读取”，**Draw Call 和 CPU 负担大幅下降**。
- **好处**：Draw Call 少、CPU–GPU 通信少、适合大量重复物体；**坏处**：只适合同 mesh、每实例差异主要是变换，且剔除/LOD/调试更复杂。
- 本课通过“行星 + 大量小行星”的实例化实现，完整走通实例数组的创建、绑定与绘制流程，便于在后续项目中有选择地使用实例化或普通绘制。

---

## 代码与文件结构

```
Lesson 19
├── lesson19_1.cpp        # 实例化主程序（行星 + 小行星）
├── 10.3.planet.vs/fs     # 行星着色器（普通绘制）
└── 10.3.asteroids.vs/fs  # 小行星着色器（实例化，使用 aInstanceMatrix）
```
