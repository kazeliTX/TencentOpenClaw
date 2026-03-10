# 10.1 Control Rig 概述与架构

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、什么是 Control Rig

```
Control Rig 是 UE5 的程序化骨骼控制系统：
  可在编辑器中为骨骼 Mesh 创建可交互的控制器（Gizmo）
  在运行时通过 Rig Graph 程序化地控制骨骼
  兼具：动画制作工具 + 运行时动画系统 的双重角色

与传统 IK 节点的区别：
  AnimBP IK 节点：功能固定，不可编程
  Control Rig：完全可编程（Rig Graph），功能无限扩展

Control Rig 的三大用途：
  1. 动画师工具：在 Sequencer 中用控制器 K 帧
  2. 运行时 IK：在 AnimGraph 中作为 Control Rig 节点运行
  3. 动画重定向：通过 IK Retargeter 将动画从一个角色迁移到另一个
```

---

## 二、架构概览

```
Control Rig 资产结构：

Control Rig Asset（.uasset）
├── Rig Hierarchy（Rig 层级）
│   ├── Bones（骨骼，来自 Skeletal Mesh）
│   ├── Controls（控制器，动画师操作的 Gizmo）
│   ├── Nulls（空节点，用于组织层级）
│   └── Spaces（空间坐标系定义）
│
└── Rig Graph（Rig 求解图）
    ├── Forward Solve Graph（正向求解）
    │   Controls → Bones（控制器驱动骨骼）
    │
    └── Backwards Solve Graph（逆向求解）
        Bones → Controls（骨骼反向驱动控制器，用于重定向）

运行时数据流：
  AnimGraph
    → [Control Rig 节点]
       → Rig Graph.ForwardSolve()
          → 计算骨骼变换
       → 输出骨骼姿势到 AnimGraph
```

---

## 三、创建 Control Rig

```
步骤 1：从 Skeletal Mesh 创建
  内容浏览器 → 右键 SkeletalMesh
  → Create → Control Rig
  命名：CR_MyCharacter

步骤 2：打开 Control Rig 编辑器
  Rig Hierarchy 面板：显示所有骨骼
  右键骨骼 → New Control（为骨骼创建控制器）

步骤 3：创建控制器
  常见控制器类型：
    Circle（圆圈）    ← 旋转控制（如关节）
    Cube（方块）      ← 移动控制（如 Root 位置）
    Sphere（球形）    ← IK 目标（如脚部 IK 目标）
    Cross（十字）     ← 方向指示

步骤 4：在 Rig Graph 中编写求解逻辑
  （详见 10.3 Rig Graph）
```

---

## 四、延伸阅读

- 📄 [10.2 Rig 层级](./02-rig-hierarchy.md)
- 🔗 [Control Rig 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-in-unreal-engine)
