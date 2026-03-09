# 5.9 Control Rig 中的 IK 求解器

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、Control Rig 简介

Control Rig 是 UE5 中最强大的过程化动画系统，可以：

```
Control Rig 的能力：
  1. 提供多种内置 IK 求解器（比 AnimBP 节点更强大）
  2. 编写可视化脚本（类似蓝图，但在工作线程运行）
  3. 在 AnimGraph 中作为节点执行（零额外线程切换）
  4. 用于制作工具（动画 + 绑定工具）

Control Rig 内置 IK 求解器：
  Limb IK:       四肢 IK（增强版 Two-Bone IK）
  Spine IK:      脊椎 IK（FABRIK 变体，专为脊椎优化）
  Full Body IK:  全身 IK
  Point at:      朝向目标
  Basic IK:      基础单链 IK
```

---

## 二、创建 Control Rig 资产

```
内容浏览器 → 右键 → Animation → Control Rig
选择 Skeleton → 命名：CR_Character_FootIK

Control Rig 编辑器：
  左侧：Rig 层级（Bones, Controls, Nulls）
  中间：3D 视口（角色预览）
  右侧：节点图（Rig Graph，类似蓝图）
```

---

## 三、使用 Spine IK 求解器

```
脊椎 IK 示例（Control Rig Rig Graph）：

// Rig Graph 节点连接（概念描述）：

[Get Transform（pelvis）]
        │
[Spine IK]
  Bone Chain:  spine_01 → spine_02 → spine_03 → neck → head
  Effector:    LookAtTarget（Control 变量，外部传入）
  Root Pin:    ✅（固定根部）
  Pole Vector: ForwardVector × 50（脊椎弯曲参考）
  Iterations:  10
        │
[Set Transforms（写回骨骼）]
```

---

## 四、在 AnimGraph 中运行 Control Rig

```
AnimGraph 节点：Control Rig

配置：
  Control Rig Class: CR_Character_FootIK（你创建的 CR 资产）

输入变量（在节点 Details 中绑定）：
  LeftFootTarget  ← AnimInstance.LeftFootIKTarget
  RightFootTarget ← AnimInstance.RightFootIKTarget
  LookAtTarget    ← AnimInstance.HeadLookAtTargetCS

// Control Rig 内部读取这些变量，执行 IK，并将结果写回骨骼
// 比 AnimBP 内置节点更灵活，可以包含复杂的自定义逻辑
```

---

## 五、Control Rig 的 Twist 和 Roll 求解器

```
Twist Solver（扭转分配器）：
  解决前臂/大腿扭转问题（第二章提到的辅助骨骼）
  
  配置：
    Source Bone: lowerarm_l（驱动骨）
    Target Bone: lowerarm_twist_01_l（被驱动骨）
    Twist Axis:  X（沿骨骼轴）
    Weight:      0.5（传递 50% 扭转量）
  
  效果：自动替代手动配置 Twist 辅助骨骼的权重
  
Roll Solver：
  处理骨骼绕自身轴旋转的分配
  类似 Twist Solver 但有更精细控制
```

---

## 六、延伸阅读

- 🔗 [Control Rig 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-in-unreal-engine)
- 🔗 [Control Rig 求解器列表](https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-solvers-in-unreal-engine)
- 🔗 [UE5 Control Rig 教程（知乎）](https://zhuanlan.zhihu.com/p/671487108)
