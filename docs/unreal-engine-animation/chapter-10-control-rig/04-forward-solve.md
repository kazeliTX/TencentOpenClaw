# 10.4 Forward Solve（FK 正向求解）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、Forward Solve 流程

```
Forward Solve = 控制器 → 骨骼（动画播放时）

典型 Forward Solve 顺序：
  1. 读取输入动画姿势（来自 AnimBP 的 Base Pose）
  2. 读取各控制器的值（动画师 K 帧的或程序化生成的）
  3. 按层级顺序计算骨骼变换（父骨骼先于子骨骼）
  4. 应用 IK（修正末端效应器位置）
  5. 输出最终骨骼变换
```

---

## 二、FK 链实现

```
脊椎 FK 实现（spine_01 → spine_02 → spine_03 → neck → head）：

设计：
  5 个 FK 控制器，每个旋转独立
  支持叠加旋转（上层旋转会影响所有子骨骼）

Rig Graph 节点（简化）：
  
  [Get Base Pose: spine_01] ─── + ─── [Get Control: ctrl_spine_01]
                                          ↓ (Local Rotation)
                              [Set Bone: spine_01 (Local)]
                                          │
  [Get Base Pose: spine_02] ─── + ─── [Get Control: ctrl_spine_02]
                                          │
                              [Set Bone: spine_02 (Local)]
                                          │
  ...（依次类推）

Base Pose 的作用：
  AnimGraph 传入的基础动画姿势
  FK 控制器在基础姿势上"叠加"旋转
  → 动画 + 程序化调整的叠加效果
```

---

## 三、混合 Base Pose 与 FK 控制

```
在 Rig Graph 中混合基础动画与程序化控制：

// 将 Base Pose 与 FK 控制器混合
// Alpha = 0：纯基础动画
// Alpha = 1：纯 FK 控制器

节点：Blend Bone Transform
  A: Get Bone Transform (spine_01, from Base Pose)
  B: Get Control Transform (ctrl_spine_01)
  Alpha: BlendAlpha (Member Variable, 0~1)
  → Set Bone Transform

应用场景：
  过场动画：Alpha = 1（动画师完全控制）
  战斗中：Alpha = 0（运行时动画驱动）
  受伤弯腰：Alpha = 0.5（基础动画 + 程序化弯腰叠加）
```

---

## 四、Full Body IK 在 Forward Solve 中

```
Full Body IK（FBIK）是 Control Rig 的核心 IK 求解器：

适合：同时控制多个末端（双手/双脚/头部）的全身 IK

配置（Forward Solve 中）：
  1. FBIK Solver 节点
     Root: pelvis
     Effectors: [ctrl_foot_l, ctrl_foot_r, ctrl_hand_l, ctrl_hand_r]
  
  2. 每个 Effector 设置：
     Position: IK 目标位置
     Rotation: IK 目标旋转
     Pull Weight: 1.0（效应器权重）
     
  3. 骨骼设置（FBIK Bone Settings）：
     所有骨骼默认自由
     可对特定骨骼设置旋转限制
     如：膝盖只允许在膝盖平面内弯曲

输出：所有骨骼的最优变换（满足所有效应器约束）
```

---

## 五、延伸阅读

- 📄 [10.6 Control Rig IK 节点详解](./06-ik-nodes.md)
- 🔗 [Full Body IK 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/full-body-ik-in-unreal-engine)
