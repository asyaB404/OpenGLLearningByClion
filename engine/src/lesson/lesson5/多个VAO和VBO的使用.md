# 多个 VAO 和 VBO 的使用

## 概述

在 Lesson 5 中，我们学习了如何使用**一个 VAO/VBO** 来渲染多个相同形状的立方体。但在实际开发中，我们经常需要渲染**不同类型的物体**（例如：立方体、球体、三角形等），它们可能有不同的顶点数据格式或不同的几何形状。这时就需要使用**多个 VAO 和 VBO**。

## 使用场景

### 场景 1：渲染不同类型的物体

当你需要同时渲染不同类型的几何体时，每种类型需要自己的 VAO/VBO：

- **立方体**：需要自己的 VAO/VBO（36 个顶点）
- **球体**：需要自己的 VAO/VBO（球体的顶点数据）
- **三角形**：需要自己的 VAO/VBO（3 个顶点）
- **平面**：需要自己的 VAO/VBO（4 个顶点）

### 场景 2：不同的顶点数据格式

即使形状相同，如果顶点数据格式不同，也需要不同的 VAO/VBO：

- **格式 A**：位置(3) + 颜色(3) = 6 个 float
- **格式 B**：位置(3) + 纹理坐标(2) = 5 个 float
- **格式 C**：位置(3) + 颜色(3) + 纹理坐标(2) + 法线(3) = 11 个 float

### 场景 3：动态更新顶点数据

如果某些物体的顶点数据需要频繁更新（例如：粒子系统、变形动画），可能需要单独的 VBO。

## 实现方式

### 方式一：为每种物体类型创建独立的 VAO/VBO

```cpp
// 定义多个 VAO 和 VBO
unsigned int cubeVAO, cubeVBO;      // 立方体的 VAO/VBO
unsigned int sphereVAO, sphereVBO;  // 球体的 VAO/VBO
unsigned int planeVAO, planeVBO;   // 平面的 VAO/VBO

// 创建立方体的 VAO/VBO
glGenVertexArrays(1, &cubeVAO);
glGenBuffers(1, &cubeVBO);
glBindVertexArray(cubeVAO);
glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
// 设置顶点属性（例如：位置 + 纹理坐标）
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);

// 创建球体的 VAO/VBO
glGenVertexArrays(1, &sphereVAO);
glGenBuffers(1, &sphereVBO);
glBindVertexArray(sphereVAO);
glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);
// 设置顶点属性（例如：位置 + 法线）
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(2);

// 创建平面的 VAO/VBO
glGenVertexArrays(1, &planeVAO);
glGenBuffers(1, &planeVBO);
glBindVertexArray(planeVAO);
glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
// 设置顶点属性
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
```

### 方式二：在渲染循环中切换 VAO

```cpp
while (!glfwWindowShouldClose(window))
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    ourShader.use();
    
    // 设置投影和视图矩阵
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                            (float)SCR_WIDTH / (float)SCR_HEIGHT, 
                                            0.1f, 100.0f);
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    ourShader.setMat4("projection", projection);
    ourShader.setMat4("view", view);
    
    // 渲染立方体
    glBindVertexArray(cubeVAO);
    for (unsigned int i = 0; i < numCubes; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);  // 立方体有 36 个顶点
    }
    
    // 渲染球体
    glBindVertexArray(sphereVAO);
    for (unsigned int i = 0; i < numSpheres; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, spherePositions[i]);
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);  // 球体的顶点数
    }
    
    // 渲染平面
    glBindVertexArray(planeVAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
    ourShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);  // 平面有 6 个顶点（2个三角形）
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}
```

## 单个 VAO/VBO vs 多个 VAO/VBO

### 对比表格

| 特性 | 单个 VAO/VBO | 多个 VAO/VBO |
|------|-------------|-------------|
| **适用场景** | 渲染多个相同形状的物体 | 渲染不同类型的物体 |
| **内存使用** | 较少（只存储一份顶点数据） | 较多（每种类型都需要存储） |
| **代码复杂度** | 简单 | 较复杂 |
| **灵活性** | 低（只能渲染一种形状） | 高（可以渲染多种形状） |
| **性能** | 高（减少 VAO 切换） | 中等（需要切换 VAO） |
| **维护性** | 容易 | 需要管理多个资源 |

### 详细对比

#### 1. 内存使用

**单个 VAO/VBO：**
```cpp
// 只存储一个立方体的数据（36 个顶点 × 5 个 float × 4 字节 = 720 字节）
float cubeVertices[36 * 5] = { /* ... */ };
unsigned int VAO, VBO;
// 可以渲染 10 个、100 个甚至 1000 个立方体，但只占用 720 字节
```

**多个 VAO/VBO：**
```cpp
// 每种类型都需要存储
float cubeVertices[36 * 5] = { /* ... */ };      // 720 字节
float sphereVertices[1000 * 6] = { /* ... */ };  // 24000 字节
float planeVertices[6 * 5] = { /* ... */ };      // 120 字节
// 总共：24840 字节
```

#### 2. 渲染性能

**单个 VAO/VBO：**
- ✅ **优势**：不需要切换 VAO，所有绘制调用使用同一个 VAO
- ✅ **优势**：GPU 可以更好地优化（所有顶点数据在同一个缓冲区）
- ✅ **优势**：减少 OpenGL 状态切换的开销

**多个 VAO/VBO：**
- ⚠️ **劣势**：需要在不同 VAO 之间切换（`glBindVertexArray()`）
- ⚠️ **劣势**：每次切换都有一定的开销
- ✅ **优势**：可以针对不同物体使用不同的优化策略

#### 3. 代码组织

**单个 VAO/VBO：**
```cpp
// 简单清晰
glBindVertexArray(VAO);
for (int i = 0; i < numObjects; i++) {
    // 设置模型矩阵
    // 绘制
}
```

**多个 VAO/VBO：**
```cpp
// 需要管理多个资源
void RenderCubes() {
    glBindVertexArray(cubeVAO);
    // 渲染立方体
}

void RenderSpheres() {
    glBindVertexArray(sphereVAO);
    // 渲染球体
}

void RenderPlanes() {
    glBindVertexArray(planeVAO);
    // 渲染平面
}
```

## 实际应用示例

### 示例：渲染一个包含多种物体的场景

```cpp
// 场景包含：
// - 10 个立方体（使用同一个 VAO）
// - 5 个球体（使用同一个 VAO）
// - 1 个地面平面（使用自己的 VAO）

// 初始化阶段
void SetupScene()
{
    // 创建立方体 VAO/VBO
    SetupCubeVAO();
    
    // 创建球体 VAO/VBO
    SetupSphereVAO();
    
    // 创建平面 VAO/VBO
    SetupPlaneVAO();
}

// 渲染阶段
void RenderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 设置投影和视图矩阵（所有物体共享）
    SetProjectionAndView();
    
    // 渲染立方体（使用立方体 VAO）
    glBindVertexArray(cubeVAO);
    for (int i = 0; i < 10; i++) {
        glm::mat4 model = CalculateCubeModelMatrix(i);
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    
    // 切换 VAO：渲染球体（使用球体 VAO）
    glBindVertexArray(sphereVAO);
    for (int i = 0; i < 5; i++) {
        glm::mat4 model = CalculateSphereModelMatrix(i);
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
    }
    
    // 切换 VAO：渲染平面（使用平面 VAO）
    glBindVertexArray(planeVAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
    ourShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
```

## VAO 切换的开销

### 性能考虑

每次调用 `glBindVertexArray()` 都会：
1. 切换 OpenGL 的状态
2. 可能需要验证 VAO 的有效性
3. 更新 GPU 的内部状态

**优化建议：**
- ✅ 按类型分组渲染：先渲染所有立方体，再渲染所有球体，而不是交替渲染
- ✅ 减少不必要的 VAO 切换：如果连续渲染相同类型的物体，不要重复绑定
- ✅ 使用实例化渲染（Instancing）：对于大量相同物体，使用实例化而不是循环

### 性能测试示例

```cpp
// ❌ 不好的做法：频繁切换 VAO
for (int i = 0; i < objects.size(); i++) {
    glBindVertexArray(objects[i].VAO);  // 每次循环都切换
    glDrawArrays(...);
}

// ✅ 好的做法：按类型分组
// 先渲染所有立方体
glBindVertexArray(cubeVAO);
for (int i = 0; i < numCubes; i++) {
    glDrawArrays(...);
}

// 再渲染所有球体
glBindVertexArray(sphereVAO);
for (int i = 0; i < numSpheres; i++) {
    glDrawArrays(...);
}
```

## 混合使用策略

在实际项目中，通常会**混合使用**两种方式：

```cpp
// 策略：相同类型的物体共享 VAO，不同类型的物体使用不同的 VAO

// 立方体：一个 VAO，多个实例
glBindVertexArray(cubeVAO);
for (int i = 0; i < numCubes; i++) {
    // 使用不同的模型矩阵
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// 球体：一个 VAO，多个实例
glBindVertexArray(sphereVAO);
for (int i = 0; i < numSpheres; i++) {
    // 使用不同的模型矩阵
    glDrawArrays(GL_TRIANGLES, 0, sphereVertexCount);
}

// 平面：一个 VAO，单个实例
glBindVertexArray(planeVAO);
glDrawArrays(GL_TRIANGLES, 0, 6);
```

## 总结

### 何时使用单个 VAO/VBO？

✅ **适合场景：**
- 渲染多个**相同形状**的物体
- 物体使用**相同的顶点数据格式**
- 追求**最佳性能**（减少状态切换）
- 代码需要**简单清晰**

### 何时使用多个 VAO/VBO？

✅ **适合场景：**
- 渲染**不同类型的物体**（立方体、球体、三角形等）
- 物体使用**不同的顶点数据格式**
- 需要**灵活性**和**可扩展性**
- 场景包含**多种几何体**

### 最佳实践

1. **按类型分组**：相同类型的物体共享一个 VAO
2. **减少切换**：在渲染循环中，按类型分组渲染，减少 VAO 切换次数
3. **合理设计**：根据实际需求选择策略，不要过度设计
4. **性能测试**：如果性能是关键，进行基准测试来验证选择

### 关键要点

- **单个 VAO/VBO**：高效、简单，适合渲染大量相同物体
- **多个 VAO/VBO**：灵活、强大，适合渲染不同类型的物体
- **混合使用**：实际项目中最常见的策略
- **性能优化**：减少 VAO 切换，按类型分组渲染

