# 5.3 FABRIK 链式 IK

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 20 分钟

---

## 一、FABRIK 适用场景

```
Two-Bone IK：只支持 2 根骨骼（大腿+小腿，上臂+前臂）
FABRIK：     支持任意数量骨骼（脊椎、尾巴、触手、绳索）

FABRIK 典型应用：
  • 脊椎 IK（spine_01 → spine_05，5 根骨骼）
  • 尾巴 IK（10~15 根骨骼）
  • 触手/触手 IK
  • 头部注视（颈部 + 头部多骨骼链）
  • 爬行动物颈部（多节）
```

---

## 二、AnimGraph 中配置 FABRIK

```
节点参数：

Tip Bone:         head           （链末端，效应器目标骨骼）
Root Bone:        spine_01       （链起点，保持固定）
Precision:        1.0            （精度，cm。越小 = 越精确但越慢）
Max Iterations:   10             （最大迭代次数）
Enable Debug Draw: false         （调试时开启）

Effector Transform:
  Translation: LookAtTargetCS   （来自 AnimBP 变量）
  Location Target Space: Component Space

Alpha: 1.0                       （IK 权重）
```

### 脊椎 Look At 配置

```
目标：让角色身体跟随相机方向倾斜（多节脊椎）

Tip Bone:   head        （头部是链末端）
Root Bone:  spine_01    （脊椎根部固定）

Effector = 相机前方某点（Component Space）
  计算方式：
    WorldTarget = Camera.Location + Camera.Forward * 300
    TargetCS    = Mesh.ComponentToWorld.Inverse().TransformPosition(WorldTarget)

约束（可选）：
  每根骨骼的旋转限制（防止身体过度弯曲）
  在 FABRIK 节点 → Bone Constraints → Per Bone Settings
    spine_01: Max Angle = 15°
    spine_02: Max Angle = 15°
    spine_03: Max Angle = 20°
    neck:     Max Angle = 30°
    head:     Max Angle = 40°
```

---

## 三、FABRIK vs Two-Bone IK 选择

```
Two-Bone IK 优先级更高（更快）：
  精确的几何解析解
  适合：手臂、腿部（固定 2 根骨骼）

FABRIK 适用于：
  骨骼数量 > 2
  不需要精确的极向控制（Pole Target）
  脊椎、尾部等弯曲链

性能对比（10 根骨骼 FABRIK vs Two-Bone IK）：
  Two-Bone：~0.01ms（解析解）
  FABRIK 10骨骼 10迭代：~0.08ms（可接受）
  FABRIK 50骨骼 20迭代：~0.5ms（需要 LOD 控制）
```

---

## 四、延伸阅读

- 🔗 [FABRIK 节点文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-fabrik-in-unreal-engine)
- 🔗 [FABRIK 原始论文](http://www.andreasaristidou.com/publications/papers/FABRIK.pdf)
