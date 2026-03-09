# 2.3 蒙皮算法：LBS vs DQS

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟
> 这是理解动画变形质量的核心知识

---

## 一、蒙皮的本质

蒙皮（Skinning）解决的问题：
> "已知骨骼的运动，如何让绑定在骨骼上的皮肤网格体跟着变形？"

```
蒙皮的输入/输出：

输入：
  • 原始顶点位置 V（模型空间，参考姿势下）
  • 每个顶点的蒙皮权重 w[i]（受哪些骨骼影响，权重之和 = 1.0）
  • 每根骨骼的当前蒙皮矩阵 M[i]（= InvBind × CurrentWorld）

输出：
  • 变形后的顶点位置 V'

公式（多骨骼加权）：
  V' = Σ(w[i] × M[i] × V)   （LBS 公式）
  
  其中 i 遍历影响该顶点的所有骨骼（通常最多 4 或 8 根）
```

---

## 二、LBS：线性混合蒙皮（Linear Blend Skinning）

### 2.1 原理

LBS 是最传统、最广泛使用的蒙皮算法。

```
LBS 核心步骤：

1. 对每根骨骼，计算 4×4 蒙皮矩阵 M[i]
2. 将矩阵按权重线性混合：M_blended = Σ(w[i] × M[i])
3. 用混合后的矩阵变换顶点：V' = M_blended × V

等价写法（展开）：
V' = w[0]×M[0]×V + w[1]×M[1]×V + w[2]×M[2]×V + ...
```

### 2.2 LBS 的致命缺陷："糖果纸扭曲"

```
问题场景：手腕旋转 180°（如翻转手掌）

骨骼 A（手腕）：旋转矩阵 R_A（初始）
骨骼 B（手腕）：旋转矩阵 R_B（旋转 180°）

处于中间的顶点（权重各 0.5）：
M_blended = 0.5 × R_A + 0.5 × R_B

问题：两个旋转矩阵的算术平均 ≠ 中间旋转！
对于 180° 旋转，矩阵平均会产生接近于 0 的矩阵
→ 几何体"塌陷"到骨骼轴上 → 看起来像被糖纸拧在一起

         正常手腕                 LBS 扭曲
    ╔════════════╗           ╔══╗    ╔══╗
    ║  圆柱形手腕 ║     →     ║  ╳   ╳  ║  （中间塌陷）
    ╚════════════╝           ╚══╝    ╚══╝
```

### 2.3 LBS 的优点

- 计算简单，GPU 友好（矩阵运算完全并行化）
- 全平台支持，历史悠久，稳定
- 适合小角度旋转的关节（大多数正常动画）

---

## 三、DQS：双四元数蒙皮（Dual Quaternion Skinning）

### 3.1 原理

DQS 用**双四元数**（Dual Quaternion）代替 4×4 矩阵来表示刚体变换（旋转 + 位移），并在双四元数空间中做插值。

```
双四元数（Dual Quaternion）：
  Q = Q_real + ε × Q_dual
  
  其中：
    Q_real = 旋转部分（普通四元数）
    Q_dual = 位移编码（与旋转耦合）
    ε = "对偶单位"（ε² = 0）

DQS 蒙皮步骤：
  1. 将每根骨骼的 4×4 矩阵转换为双四元数 DQ[i]
  2. 在双四元数空间中加权混合（DQ_blended = Σ(w[i] × DQ[i])）
  3. 规范化混合后的双四元数
  4. 将双四元数转回矩阵，变换顶点

关键区别：
  LBS：混合矩阵（矩阵元素线性插值，破坏正交性）
  DQS：混合双四元数（旋转插值更正确，保持刚性）
```

### 3.2 DQS 的效果

```
手腕旋转 180° 对比：

LBS：                DQS：
  ╔══╗  ╔══╗           ╔════════════╗
  ║  ╳  ╳  ║    →      ║  自然扭曲   ║  （保持体积）
  ╚══╝  ╚══╝           ╚════════════╝

DQS 在扭曲关节处保持体积，视觉质量明显更好
```

### 3.3 DQS 的缺陷："凸起"伪影（Bulging）

```
问题场景：当多根骨骼旋转方向相反时

  骨骼 A: 向左旋转 90°
  骨骼 B: 向右旋转 90°
  权重各 50%

DQS 会在关节处产生异常"膨胀"（Bulging），
是因为双四元数混合在某些极端情况下插值路径穿过了"膨胀区域"

解决方案：
  • 在这些特殊关节上强制使用 LBS
  • 减小相对旋转量（通过动画师控制）
  • UE 允许按材质/顶点选择算法
```

---

## 四、UE 中的配置

### 4.1 全局切换

```
项目设置：
Project Settings → Engine → Rendering → Skinning
  • Default Skinning Mode：Linear Blend / Dual Quaternion / Blend

Build Settings（针对特定 Mesh）：
  SkeletalMesh → Details → Skin Weights → Skinning Method
  • Blend：基于权重阈值混合 LBS 和 DQS（推荐）
    配套：Blend Threshold（大于此权重的关节用 DQS，小于的用 LBS）
```

### 4.2 逐材质配置

```cpp
// 通过材质参数在 Shader 中控制（高级用法）
// UE 的 GPU Skinning Shader 会根据 SkinningMode 选择分支：
//   0 = LBS（默认，最快）
//   1 = DQS（高质量，略慢）
//   2 = Blend（混合模式）

// 运行时切换（需要自定义 GameUserSettings）
GetMesh()->SetSkinWeightProfile(FName("HighQuality"));
```

---

## 五、GPU Skinning vs CPU Skinning

```
GPU Skinning（默认，推荐）：
  • 蒙皮在 Vertex Shader 中完成
  • 骨骼矩阵作为 Uniform Buffer 传给 GPU
  • 并行处理所有顶点，极快
  • 适用于：几乎所有情况

CPU Skinning（特殊场景）：
  • 蒙皮在 CPU 上完成，结果传给 GPU
  • 使用场景：
    - 需要在 CPU 读取变形后的顶点（如精确物理碰撞）
    - 某些低端平台不支持足够多的骨骼矩阵 Uniform
  • 开启方式：
    Mesh->bCPUSkinning = true;  // 通常不推荐

骨骼数量上限（GPU Skinning）：
  • 默认：75 根骨骼（单个 Draw Call）
  • 可调整：ProjectSettings → Rendering → Max GPU Skin Bones（最高 256）
  • 超出上限时：UE 自动分割为多个 Draw Call
```

---

## 六、蒙皮权重精度

```
UE 支持两种权重精度：

4 骨骼权重（4 Bone Influence，默认）：
  每个顶点最多受 4 根骨骼影响
  权重精度：8-bit（0-255）
  内存：4 个索引 + 4 个权重 = 8 bytes/顶点

8 骨骼权重（8 Bone Influence）：
  每个顶点最多受 8 根骨骼影响
  权重精度：16-bit
  内存：8 个索引 + 8 个权重 = 24 bytes/顶点
  适用：手部、面部等需要更多骨骼参与的精细区域

配置：
  SkeletalMesh → Details → Mesh → Use Full Precision UVs
  SkeletalMesh → Details → LOD Settings → Max Bone Influences = 4 或 8
```

---

## 七、学术参考

| 论文 | 作者 | 年份 | 链接 |
|------|------|------|------|
| Skinning with Dual Quaternions | Kavan et al. | 2008 | https://users.cs.utah.edu/~ladislav/kavan08skinning/kavan08skinning.pdf |
| Linear Blend Skinning | 综述 | 经典 | 教科书内容 |
| Smooth Skinning Decomposition with Rigid Bones | Le & Deng | 2012 | ACM SIGGRAPH Asia |
| Bounded Biharmonic Weights | Jacobson et al. | 2011 | SIGGRAPH |

---

## 八、延伸阅读

- 📄 [2.4 蒙皮权重优化实践](./04-skin-weights.md)
- 🔗 [DQS 原始论文（PDF）](https://users.cs.utah.edu/~ladislav/kavan08skinning/kavan08skinning.pdf)
- 🔗 [UE Rendering: Skeletal Mesh Skinning](https://dev.epicgames.com/documentation/en-us/unreal-engine/skeletal-mesh-rendering-paths-in-unreal-engine)
