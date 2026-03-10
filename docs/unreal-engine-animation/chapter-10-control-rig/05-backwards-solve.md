# 10.5 Backwards Solve（动画重定向基础）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、Backwards Solve 的作用

```
Backwards Solve = 骨骼 → 控制器（逆向）

用途：
  1. IK Rig 重定向：将 A 骨骼的动画迁移到 B 骨骼
     → Backwards Solve 先将 A 骨骼动画"提取"为控制器值
     → Forward Solve 再将控制器值"应用"到 B 骨骼

  2. Sequencer 动画导入：
     将骨骼动画数据"反解"为 Control 值
     便于动画师在 Sequencer 中编辑

动画重定向流程：
  Source Mesh 动画（A_Walk.fbx，Mannequin 骨骼）
        ↓ Backwards Solve
  IK Controls（中间表示）
        ↓ Forward Solve
  Target Mesh 骨骼（Quinn，不同身材）
  → 自动适配不同骨骼比例的同一动画
```

---

## 二、Backwards Solve 实现

```
Backwards Solve Graph 的逻辑是 Forward Solve 的逆：

Forward:  ctrl_hand_ik (position) → FABRIK → bone_hand_l
Backward: bone_hand_l (position) → Set Control ctrl_hand_ik

// 简单手部 IK 的 Backwards Solve：
[Get Bone Transform: hand_l (Global)]
  → [Set Control: ctrl_hand_ik (Position, Global)]

[Get Bone Transform: hand_l (Rotation)]
  → [Set Control: ctrl_hand_rot (Rotation, Local)]

注意：
  不是所有骨骼都需要 Backwards Solve
  只有"驱动骨骼"的控制器需要
  FK 控制器通常不需要精确 Backwards Solve
```

---

## 三、IK Rig 资产

```
IK Rig 是专门用于重定向的资产（区别于 Control Rig）：

创建：
  内容浏览器 → 右键 SkeletalMesh → Create → IK Rig
  命名：IKRig_Mannequin

IK Rig 配置：
  Root Bone: root（全身根骨骼）
  
  IK Goals（IK 目标，对应关键末端）：
    足部：IKGoal_Foot_L, IKGoal_Foot_R
    手部：IKGoal_Hand_L, IKGoal_Hand_R
    头部：IKGoal_Head
    骨盆：IKGoal_Pelvis（保持躯干高度）
  
  Retarget Chains（重定向骨骼链）：
    Spine Chain: spine_01 → spine_05
    Left Arm:    clavicle_l → hand_l
    Right Arm:   clavicle_r → hand_r
    Left Leg:    thigh_l → foot_l
    Right Leg:   thigh_r → foot_r

IK Rig 不执行运行时计算
仅作为 IK Retargeter 的"描述文件"使用
```

---

## 四、延伸阅读

- 📄 [10.8 IK Retargeter 详解](./08-anim-retarget.md)
- 🔗 [IK Rig 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-in-unreal-engine)
- 🔗 [IK Retargeter 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-animation-retargeting-in-unreal-engine)
