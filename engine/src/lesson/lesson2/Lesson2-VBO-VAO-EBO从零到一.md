# Lesson 2：VBO、VAO、EBO 从零到一

## 📚 目录
- [概述](#概述)
- [第 0 步：GPU 要画什么？](#第-0-步gpu-要画什么)
- [第 1 步：顶点数据从哪来、放哪去？](#第-1-步顶点数据从哪来放哪去)
- [第 2 步：VBO——把顶点数据放进 GPU](#第-2-步vbo把顶点数据放进-gpu)
- [第 3 步：GPU 怎么“读”这批数据？顶点属性](#第-3-步gpu-怎么读这批数据顶点属性)
- [第 4 步：VAO——把“怎么读”记下来](#第-4-步vao把怎么读记下来)
- [第 5 步：EBO——用索引少传顶点](#第-5-步ebo用索引少传顶点)
- [扩展：GPU 实例化在这块的作用](#扩展gpu-实例化在这块的作用)
- [整体流程：从创建到一次绘制](#整体流程从创建到一次绘制)
- [VBO / VAO / EBO 对照表](#vbo--vao--ebo-对照表)
- [常见问题](#常见问题)
- [总结](#总结)

---

## 概述

在 OpenGL 里画三角形、矩形或任意几何体，都需要把**顶点数据**交给 GPU，再告诉 GPU **如何解释这些数据**、**按什么顺序画**。  
**VBO、VAO、EBO** 就是这三件事在 OpenGL 里的具体实现：

| 缩写 | 全称 | 一句话作用 |
|------|------|-------------|
| **VBO** | Vertex Buffer Object（顶点缓冲对象） | 在 **GPU 显存**里存**顶点数据**（位置、颜色、UV 等） |
| **VAO** | Vertex Array Object（顶点数组对象） | 存**“怎么读”顶点数据的配置**（属性格式、步长、偏移等），并关联 EBO |
| **EBO** | Element Buffer Object（索引缓冲对象） | 在 GPU 里存**顶点索引**，用“第几个顶点”来画三角形，**复用顶点、少传数据** |

下面从“GPU 要画什么”开始，一步一步说明它们各自干什么、为什么需要、怎么配合使用。

---

## 第 0 步：GPU 要画什么？

OpenGL 画的是**由顶点组成的图元**（点、线、三角形等）。  
最基础的是**三角形**：每 3 个顶点组成一个三角形。

- 例如画一个**矩形**：可以拆成 **2 个三角形**，共 **6 个顶点**（每个三角形 3 个顶点）。
- 每个顶点至少要有**位置**（x, y, z），后面还会加颜色、法线、纹理坐标等。

所以：

1. 我们要准备一批**顶点数据**（比如很多个 float：x, y, z, x, y, z, ...）。
2. 这些数据最终要交给 **GPU**，因为画图是在 GPU 上完成的。
3. GPU 还要知道：**每几个数算一个顶点？**、**先画哪几个顶点？**（顺序）

**第 0 步的结论**：  
画图 = 把**顶点数据**给 GPU + 告诉 GPU **数据格式**和**绘制顺序**。  
VBO / VAO / EBO 就是为这三件事服务的。

---

## 第 1 步：顶点数据从哪来、放哪去？

顶点数据一开始在 **CPU 端**（比如 C++ 里的一个 `float vertices[]` 数组）。  
但**真正画图的是 GPU**，所以：

- 数据必须**拷贝到 GPU 能直接访问的内存**里（显存）；
- 不能每帧从 CPU 再传一大块过去，那样太慢。

所以 OpenGL 提供了**缓冲区（Buffer）**：  
在显存里划一块区域，专门放某类数据。  
把顶点数据一次性（或按需）拷贝进这块区域，之后绘制时 GPU 直接从显存读。

- **用来存顶点数据的缓冲区**，就叫 **VBO**（Vertex Buffer Object）。  
- 也就是说：**顶点数据从 CPU 来，放到 GPU 的 VBO 里。**

---

## 第 2 步：VBO——把顶点数据放进 GPU

### VBO 是什么？

**VBO（Vertex Buffer Object）** = 一块在 **GPU 显存**里的缓冲区，专门用来存**顶点相关数据**（位置、颜色、法线、UV 等）。

- **作用**：把 CPU 上的顶点数组拷贝到显存，供绘制时使用。
- **好处**：数据在 GPU 本地，访问快；不需要每帧从 CPU 再传。

### 怎么用？

1. **创建**：`glGenBuffers(1, &VBO)` 得到一个缓冲区 ID。
2. **绑定**：`glBindBuffer(GL_ARRAY_BUFFER, VBO)` 表示“接下来对 `GL_ARRAY_BUFFER` 的操作都作用在这个 VBO 上”。
3. **上传数据**：`glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW)` 把 `data` 拷贝进当前绑定的 VBO。

```cpp
float vertices[] = {
     0.5f,  0.5f, 0.0f,   // 顶点 0
     0.5f, -0.5f, 0.0f,   // 顶点 1
    -0.5f, -0.5f, 0.0f,   // 顶点 2
    -0.5f,  0.5f, 0.0f    // 顶点 3
};
glGenBuffers(1, &VBO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
```

此时：**顶点数据已经在 GPU 的 VBO 里了**。  
但 GPU 还不知道“每 3 个 float 是一个顶点的 x,y,z”，这就需要**顶点属性**。

---

## 第 3 步：GPU 怎么“读”这批数据？顶点属性

VBO 里是一长串数字，GPU 需要知道：

- 从哪一字节开始读？
- 每个顶点占多少字节（步长）？
- 这个属性是几维的（3 个 float = vec3）？
- 对应着色器里的哪个 `layout(location = N) in ...`？

这些信息通过 **顶点属性（Vertex Attribute）** 来配置，在 OpenGL 里用 **`glVertexAttribPointer`** 设置。

### glVertexAttribPointer 做什么？

它表示：“**当前绑定在 `GL_ARRAY_BUFFER` 上的 VBO 里，第 `index` 号属性是这样取的：**”

- **location**（第 1 个参数）：对应着色器里 `layout(location = 0) in vec3 aPos;` 的 0。
- **size**：该属性有几个分量（如 3 表示 x,y,z）。
- **type**：分量类型（如 `GL_FLOAT`）。
- **stride**：相邻两个**顶点**之间隔多少字节（例如只有位置时，一个顶点 3 个 float，stride = 3 * sizeof(float)）。
- **offset**：这个属性在该顶点内、从顶点起始位置偏移多少字节（例如第二个属性从 12 字节开始，就填 `(void*)12`）。

配置完后，还要 **`glEnableVertexAttribArray(index)`**，这个属性才会被传入着色器。

### 重要一点：配置的是“当前绑定的 VBO”

`glVertexAttribPointer` 不会把“用哪个 VBO”记下来，它只是根据**当前**绑定的 `GL_ARRAY_BUFFER` 来解析。  
所以通常流程是：先 `glBindBuffer(GL_ARRAY_BUFFER, VBO)`，再 `glVertexAttribPointer(...)`。  
而“当前绑定的是哪个 VBO”**不会被自动保存**，所以需要一个东西把**整套顶点属性配置**记下来，方便之后绘制时一键恢复——这就是 **VAO**。

---

## 第 4 步：VAO——把“怎么读”记下来

### VAO 是什么？

**VAO（Vertex Array Object）** = 一个**状态容器**，用来保存：

- 每个顶点属性的 **`glVertexAttribPointer` 配置**；
- 每个顶点属性的 **启用/禁用**（`glEnableVertexAttribArray`）；
- 以及**当前绑定在 `GL_ELEMENT_ARRAY_BUFFER` 上的 EBO**（见下一步）。

注意：下面会说明 VAO 和 VBO 的“存”的关系，以及为什么常说“VAO 不存 VBO”。

### 为什么需要 VAO？

- 绘制时只要 **`glBindVertexArray(VAO)`**，就能恢复：用哪些属性、步长/偏移、以及用哪个 EBO。
- 不用每次 draw 前再绑 VBO、再调一遍 `glVertexAttribPointer`，代码更清晰，也符合现代 OpenGL 的用法（Core Profile 要求使用 VAO）。

### 怎么用？

1. **创建**：`glGenVertexArrays(1, &VAO)`。
2. **绑定**：`glBindVertexArray(VAO)`。之后对顶点属性的设置都会**记录进这个 VAO**。
3. 在 VAO 绑定期间：绑定 VBO → `glVertexAttribPointer` → `glEnableVertexAttribArray`；若用索引绘制，再绑定 EBO（见下）。
4. 配置完后可以 `glBindVertexArray(0)` 解绑；绘制时再 `glBindVertexArray(VAO)` 即可。

```cpp
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

glBindBuffer(GL_ARRAY_BUFFER, VBO);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);

// EBO 在下面绑定后，也会被 VAO 记住
```

所以：**VAO 用来干嘛？** —— 把“顶点数据怎么读”和“用哪个 EBO”记下来，绘制时绑定 VAO 就恢复整套状态。

### VAO 和 VBO 到底有没有“一起存”？

你可能会想：**VAO 和 VBO 都是和顶点相关的，为什么不能一起存？**

准确地说，**VAO 会按“每个顶点属性”记下“用哪块 buffer”**，只是**不会记“当前全局绑定的那块 VBO”**。区别在这两个层面：

1. **按属性存的 buffer（VAO 里是有的）**  
   当你调用 `glVertexAttribPointer(0, ...)` 时，**当时**绑定在 `GL_ARRAY_BUFFER` 上的那块 buffer，会被记在**这个属性（location 0）** 的名下，成为 VAO 状态的一部分。也就是说：**每个顶点属性在 VAO 里都存了“自己从哪块 VBO 读”**。所以绘制时你只绑 VAO、不绑 VBO，GPU 也能知道去哪块 VBO 取数——因为 VAO 里已经按属性记好了。

2. **“全局当前 VBO”不会存**  
   OpenGL 里还有一个**全局**状态叫“当前绑定到 `GL_ARRAY_BUFFER` 的 buffer”。这个**全局绑定**不属于 VAO 的状态。所以 `glBindVertexArray(VAO)` 时，不会恢复“当前 GL_ARRAY_BUFFER 是哪个”——但你也不需要恢复它，因为每个属性要读哪块 buffer 已经存在 VAO 里了。

**为什么要设计成“按属性存 buffer”，而不是“VAO 里存一个 VBO”？**

- 一个 VAO 可以**多个属性来自不同的 VBO**（例如 location 0 用 VBO1 存位置，location 1 用 VBO2 存颜色）。所以“和顶点相关的数据”不是一块 VBO，而是**每个属性一块**。VAO 存的是“每个属性怎么读、从哪块 buffer 读”，这样才支持多 VBO。
- EBO 不同：绘制时**只能有一个**“用哪份索引”的约定，所以 EBO 是**全局一个**，OpenGL 就把它算进 VAO 状态里，绑 VAO 时会一起恢复。

**一句话**：VAO 和“顶点用哪块 buffer”是一起存的，但是**按每个顶点属性分别存**，不是存一个全局 VBO；所以绘制时只绑 VAO 就够，不需要再绑 VBO。

---

## 第 5 步：EBO——用索引少传顶点

### 为什么要索引？

画一个矩形需要 2 个三角形 = 6 个顶点。如果直接存 6 个顶点，**左下、右上**会被存两遍。  
用**索引**可以只存 4 个顶点，再用一个“索引数组”说明：  
第一个三角形用顶点 0、1、3，第二个用 1、2、3——这样顶点 1 和 3 被复用，省内存也省带宽。

### EBO 是什么？

**EBO（Element Buffer Object）** = 一块在 GPU 上的缓冲区，专门存**顶点索引**（通常是 `unsigned int`）。  
绘制时用 **`glDrawElements`**：按 EBO 里的顺序，“第几个顶点、第几个顶点、第几个顶点……”组成三角形。

### 和 VAO 的关系（重要）

- 绑定 EBO 要用 **`glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO)`**。
- **EBO 的绑定状态会被记录进当前绑定的 VAO**（和 VBO 不同！）。
- 所以：在绑定 VAO 之后绑定 EBO，以后只要 `glBindVertexArray(VAO)`，就会自动恢复“用这个 EBO 画”，不需要再手动绑 EBO。

```cpp
glBindVertexArray(VAO);
// ... 配置 VBO 和顶点属性 ...
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
// 不要解绑 EBO；解绑 VAO 即可
glBindVertexArray(0);
```

绘制时：

```cpp
glBindVertexArray(VAO);
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);  // 用 VAO 里记录的 EBO
```

更多细节见同目录下的 **《为什么 VAO 能自动绑定 EBO？》**。

---

## 扩展：GPU 实例化在这块的作用

前面讲的都是“画一份几何”：一个 VBO 存顶点，一个 EBO 存索引，一次 `glDrawElements` 画出一个物体。  
若要**用同一份 mesh 画很多份**（例如一片森林里的树、一堆小行星），可以用 **GPU 实例化（Instancing）**：还是同一套 VBO/VAO/EBO，但增加**每实例数据**（如每个物体的模型矩阵），一次 **`glDrawElementsInstanced`** 就画出 N 个物体。  
实例化仍然完全建立在 VBO/VAO 这一套机制之上，只是多用了“**按实例取数据**”的顶点属性。

### 实例化在 VBO/VAO 里的体现

- **顶点数据**：照旧用一个 VBO 存 mesh 的顶点（位置、法线、UV 等），属性用 `glVertexAttribPointer` 配置，**不区分顶点还是实例**。
- **每实例数据**：再开一个 **VBO**（或同一 VBO 的另一段），存 N 份“每实例”数据，例如 N 个 `mat4` 模型矩阵。
- **顶点属性**：对“每实例”的数据，同样用 **`glVertexAttribPointer`** 把矩阵拆成 4 个 `vec4`（占 4 个 location），配置在**当前绑定的 VAO** 下。
- **关键**：对这些 location 调用 **`glVertexAttribDivisor(location, 1)`**。  
  - **divisor = 0**（默认）：该属性是**每顶点**的，每个顶点取一次。  
  - **divisor = 1**：该属性是**每实例**的，每个实例取一次，该实例内的所有顶点共用这份值。  
  这样 GPU 画第 i 个实例时，会从“每实例 VBO”的第 i 份数据里取值（例如第 i 个矩阵），顶点着色器里就能用 `in mat4 aInstanceMatrix` 得到当前实例的变换。
- **VAO**：这些“每实例”的 attribute 配置和 divisor 也会被 **VAO 记住**，绘制时只绑 VAO 就能恢复，和普通顶点属性一样。
- **绘制**：用 **`glDrawElementsInstanced(GL_TRIANGLES, indexCount, type, offset, instanceCount)`** 或 `glDrawArraysInstanced`，一次调用画出 `instanceCount` 个实例。

所以：**实例化 = 同一 VAO（同一套 mesh + 同一套顶点属性配置）+ 多一个 VBO 存每实例数据 + 用 divisor=1 的顶点属性从该 VBO 里按实例取数 + Instanced Draw 一次画 N 个。**

### 小结：和 VBO/VAO/EBO 的关系

| 概念 | 在实例化里的角色 |
|------|------------------|
| **VBO** | 仍然负责把数据放进 GPU：一个 VBO 存 mesh 顶点，另一个 VBO（或同一 buffer 另一段）存每实例矩阵等；都是 `GL_ARRAY_BUFFER`。 |
| **顶点属性** | 多了一类“每实例”属性：同样用 `glVertexAttribPointer` 配置，但用 **`glVertexAttribDivisor(loc, 1)`** 表示按实例取。 |
| **VAO** | 同时记录“每顶点”和“每实例”的顶点属性配置（以及 EBO）；绑一次 VAO，两种数据源都生效。 |
| **EBO** | 不变，还是存三角形索引；`glDrawElementsInstanced` 仍然用同一套索引画每个实例。 |

实例化没有引入新的对象类型，只是**在现有 VAO 上多绑一个 VBO、多配几个 divisor=1 的顶点属性**，再用 Instanced Draw。  
详细实现见 **Lesson 19：实例化渲染**。

---

## 整体流程：从创建到一次绘制

把上面串起来，就是 Lesson 2 里从 0 到 1 的完整流程。

### 初始化阶段（只做一次）

1. **准备数据**：CPU 上的 `vertices[]`、`indices[]`。
2. **创建并绑定 VAO**：`glGenVertexArrays` → `glBindVertexArray(VAO)`。
3. **创建 VBO，上传顶点**：`glGenBuffers` → `glBindBuffer(GL_ARRAY_BUFFER, VBO)` → `glBufferData(...)`。
4. **配置顶点属性**：`glVertexAttribPointer(0, ...)`、`glEnableVertexAttribArray(0)`（在 VAO 绑定状态下）。
5. **创建 EBO，上传索引**：`glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO)` → `glBufferData(...)`（EBO 会被 VAO 记住）。
6. 可选：`glBindBuffer(GL_ARRAY_BUFFER, 0)`；然后 `glBindVertexArray(0)`。

### 渲染循环（每帧）

1. `glUseProgram(shaderProgram)`；
2. `glBindVertexArray(VAO)`（恢复顶点属性配置 + EBO）；
3. `glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0)`。

不需要再绑 VBO 或 EBO，VAO 里已经记录了“怎么读顶点”和“用哪个 EBO”。

---

## VBO / VAO / EBO 对照表

| 对象 | 存什么 | 作用 | 是否被 VAO 记录 |
|------|--------|------|------------------|
| **VBO** | 顶点数据（位置、颜色、UV 等） | 把顶点数据放在 GPU，供绘制时读 | 否（只记录“怎么读”的配置） |
| **VAO** | 顶点属性配置 + EBO 绑定 | 绘制时一次绑定就恢复整套顶点状态 | — |
| **EBO** | 顶点索引（第几个顶点） | 用索引画三角形，复用顶点 | 是（绑定在 VAO 激活时会被保存） |

---

## 常见问题

**Q：没有 VAO 可以画吗？**  
在 Core Profile 下，必须至少绑定一个 VAO，否则绘制会出问题。所以现代 OpenGL 习惯是：始终创建并绑定 VAO，再配置 VBO/EBO。

**Q：一个 VAO 可以对应多个 VBO 吗？**  
可以。不同 `location` 的顶点属性可以来自不同的 VBO：先绑 VBO1，配置 location 0；再绑 VBO2，配置 location 1，这些配置都会记在同一个 VAO 里。

**Q：VBO 和 EBO 可以共用一个 buffer 吗？**  
可以但不推荐。通常 VBO 专门存顶点数据，EBO 专门存索引，逻辑清晰。

**Q：为什么说“不要解绑 EBO”？**  
因为 EBO 的绑定是存在 VAO 里的。若在配置时先绑 EBO 再 `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)`，会把“无 EBO”记进 VAO，之后绘制就错了。正确做法是绑好 EBO 后直接解绑 VAO，让 VAO 记住当前 EBO。

**Q：为什么 VAO 和 VBO 不能“一起存”？**  
VAO 其实会存“每个顶点属性从哪块 buffer 读”——你调用 `glVertexAttribPointer` 时，当时绑在 `GL_ARRAY_BUFFER` 上的那块 VBO 就记在这个属性名下，属于 VAO 状态。所以绘制时只绑 VAO 就够，不需要再绑 VBO。  
“不一起存”指的是：**全局**的“当前 GL_ARRAY_BUFFER 是谁”不会存进 VAO（绑 VAO 不会恢复这个全局绑定）。这样设计是因为一个 VAO 里可以有多个属性，各自来自不同的 VBO，所以是按**每个属性**记一块 buffer，而不是 VAO 只记一个 VBO。详见上文「第 4 步」里的「VAO 和 VBO 到底有没有一起存？」。

---

## 总结

- **VBO**：在 GPU 里存顶点数据；**用来干嘛** = 把顶点从 CPU 搬到显存，供绘制用。
- **顶点属性（glVertexAttribPointer）**：告诉 GPU 从 VBO 里**怎么读**（几维、步长、偏移）。
- **VAO**：把“怎么读”的配置和**当前 EBO** 记下来；**用来干嘛** = 绘制时绑一次 VAO 就恢复整套顶点状态。
- **EBO**：在 GPU 里存顶点索引；**用来干嘛** = 用索引画三角形，复用顶点、减少数据量。

从 0 到 1 就是：**数据在 CPU → 用 VBO 搬到 GPU → 用顶点属性说明怎么读 → 用 VAO 记住这套配置和 EBO → 用 EBO 按索引画三角形。**  
Lesson 2 的代码就是这一套流程的完整实现。

**GPU 实例化**仍然用同一套 VBO/VAO/EBO：再多一个 VBO 存“每实例”数据（如模型矩阵），用 **`glVertexAttribDivisor(location, 1)`** 把这些属性设为按实例取，VAO 会一并记住；绘制时用 **`glDrawElementsInstanced`** 一次画出 N 个实例。详见上文「扩展：GPU 实例化在这块的作用」与 Lesson 19。
