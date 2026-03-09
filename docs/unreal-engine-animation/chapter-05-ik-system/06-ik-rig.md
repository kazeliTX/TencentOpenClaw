# 5.6 IK Rig 编辑器完全指南

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、IK Rig 的作用

IK Rig 是 UE5 引入的可视化 IK 配置工具，主要用于：

```
1. 动画重定向的骨骼链定义
   源骨骼 ↔ 目标骨骼的映射关系
   由 IK Retargeter 使用

2. Full Body IK 的求解器配置
   定义哪些骨骼参与 FBIK
   配置 Effector 和约束

3. 运行时 IK 蓝图（IK Rig Blueprint）
   在 AnimGraph 中作为节点运行
   提供比 AnimBP 内置节点更强大的 IK 能力
```

---

## 二、创建 IK Rig

```
步骤：
1. 内容浏览器 → 右键 → Animation → IK Rig
   选择 Skeleton → 命名：IKR_Mannequin

2. IK Rig 编辑器界面：
   ┌─────────────────────────────────────────────────┐
   │ 骨骼树（左）   │   3D 视口   │  IK 链列表（右）  │
   │               │             │  Retarget Chains  │
   │ 选择骨骼       │  角色预览   │  Solver 配置      │
   └─────────────────────────────────────────────────┘
```

---

## 三、配置重定向链（Retarget Chains）

```
Retarget Chains 定义 IK Rig 用于重定向的骨骼链组：

1. 在 IK Rig 编辑器 → Retarget Chains 面板
2. 点击 + 添加链：
   
   链名称       起始骨骼      末端骨骼     IK目标骨骼
   ─────────────────────────────────────────────────
   Root         root          root          -
   Spine        pelvis        spine_03      -
   Head         neck          head          -
   LeftArm      clavicle_l    hand_l        ik_hand_l
   RightArm     clavicle_r    hand_r        ik_hand_r
   LeftLeg      thigh_l       foot_l        ik_foot_l
   RightLeg     thigh_r       foot_r        ik_foot_r

3. 这些链将在 IK Retargeter 中被映射到目标骨骼
```

---

## 四、配置 FBIK Solver

```
Full Body IK Solver（FBIK）配置：

1. 在 IK Rig 编辑器 → Solvers 面板
2. 添加 Solver → Full Body IK
3. 在骨骼树中：
   右键骨骼 → Add Effector（添加效应器）
   
   推荐添加效应器：
   • pelvis：骨盆（Root Behavior: Pinned）
   • foot_l/foot_r：双脚（脚步 IK）
   • hand_l/hand_r：双手（手部 IK）
   • head：头部（注视 IK）

4. 骨骼设置（Bone Settings）：
   右键骨骼 → Add Settings
   Stiffness：骨骼抵抗 IK 变形的程度（0=完全跟随，1=完全固定）
   
   典型设置：
   pelvis:     Stiffness 0.6（骨盆有一定刚性）
   spine_01:   Stiffness 0.3（脊椎较柔软）
   spine_03:   Stiffness 0.2
   clavicle:   Stiffness 0.8（锁骨较固定）
```

---

## 五、IK Rig Blueprint（运行时 IK）

```
IK Rig Blueprint 允许在 AnimGraph 中使用 IK Rig：

1. 在 IK Rig 中配置好 Solver 和 Effector
2. AnimBP 中添加节点：IK Rig（搜索 "IK Rig"）
3. 设置：IK Rig Asset → 你的 IKR_Mannequin

4. 通过变量连接 Effector 目标（在 AnimGraph 节点 Details 中设置）

优势：
  比 AnimBP 内置节点更强大（Full Body IK）
  可以定义复杂约束（旋转限制、优先级）
  统一管理所有 IK 目标
```

---

## 六、延伸阅读

- 📄 [5.7 Full Body IK](./07-full-body-ik.md)
- 📄 [5.8 IK Retargeter 动画重定向](./08-ik-retargeter.md)
- 🔗 [IK Rig 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-ik-rig)
- 🔗 [IK Rig Blueprint](https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-blueprints)
