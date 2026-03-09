# 3.3 AnimGraph 节点系统

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、AnimGraph 的本质

AnimGraph 是一个**节点图求值器**：每帧从输出节点（Output Pose）开始，逆向递归求值所有连接的节点，最终得到一个 `FPoseContext`（骨骼姿势）。

```
求值方向（反向传播）：

Output Pose ← Final Animation Pose
                   │
           ← Layered Blend Per Bone
           /                        \
  ← Blend Space（下半身）      ← Aim Offset（上半身叠加）
       │                                   │
  ← State Machine                   ← AnimSequence Player
```

---

## 二、核心节点分类

### 2.1 姿势源节点（Pose Sources）

| 节点 | 说明 | 常用场景 |
|------|------|---------|
| Sequence Player | 播放单个 AnimSequence | 简单循环动画 |
| Sequence Evaluator | 按时间位置采样（不自动推进）| 手动控制播放位置 |
| Blend Space Player | 播放 BlendSpace | 移动混合（速度/方向） |
| Pose Asset | 从 PoseAsset 混合姿势 | 面部表情、IK 辅助姿势 |
| Reference Pose | 返回参考姿势（A-Pose/T-Pose） | 测试、IK 起始姿势 |
| State Machine | 状态机输出 | 角色主动画逻辑 |

### 2.2 混合节点（Blend Nodes）

| 节点 | 说明 |
|------|------|
| Blend | 两个姿势按 Alpha 混合 |
| Blend Poses by bool | 根据 bool 值选择姿势 |
| Blend Poses by int | 根据 int 值选择多个姿势之一 |
| Layered Blend Per Bone | 按骨骼分层混合（上/下半身分离） |
| Blend Multi | 多个姿势加权混合 |

### 2.3 附加姿势节点（Additive）

| 节点 | 说明 |
|------|------|
| Apply Additive | 叠加一个 Additive 姿势到 Base 上 |
| Apply Mesh Space Additive | 在 Mesh Space 叠加（Aim Offset 用这个）|

### 2.4 骨骼控制节点

| 节点 | 说明 |
|------|------|
| Two Bone IK | 双骨骼 IK（手臂/腿部）|
| FABRIK | 链式 IK |
| Look At | 骨骼注视目标 |
| Transform Modify Bone | 手动修改骨骼变换 |
| Copy Bone | 复制骨骼变换 |
| Spring Controller | 弹簧追踪（次级运动）|

---

## 三、Layered Blend Per Bone（上下半身分离）

这是角色动画中最常见的混合结构之一：

```
用途：下半身播放运动动画，上半身播放战斗动画

节点配置：
  Layered Blend Per Bone
    Base Pose:    ← 下半身动画（运动 State Machine）
    Blend Pose 0: ← 上半身动画（瞄准/攻击）
    Layer Setup:
      [0] Branch Filters:
          Bone Name: spine_03
          Blend Depth: -1（骨骼深度，-1 = 包含所有子骨骼）
          Mesh Space Rotation Blend: ✅（Aim Offset 必须勾选）
      Alpha: 1.0（完全使用上半身动画）

常见问题：
  问题：上半身动画腰部有明显接缝
  解决：Blend Depth 改为 1（脊椎渐变），或在 spine_01/02 做渐进过渡
```

---

## 四、Blend Space Player 配置

```
Blend Space（混合空间）用于在多个动画之间根据参数平滑过渡：

常用配置（8 方向移动）：
  BlendSpace 资产：
    X 轴：Speed（0~600，步行~跑步）
    Y 轴：Direction（-180~180，前后左右）
  
  采样点布局（示意）：
    Speed\Dir  -180   -90    0    90   180
      600     [↙跑] [←跑] [↑跑] [→跑] [↙跑]
      300     [↙走] [←走] [↑走] [→走] [↙走]
        0     [待机] [待机] [待机] [待机] [待机]

在 AnimGraph 中：
  节点：Blend Space Player
    Blend Space: BS_LocomotionDirectional
    X (Speed):   Speed 变量
    Y (Direction): Direction 变量
```

---

## 五、Output Pose 的数据流

```
AnimGraph 节点求值的实际数据流：

每个节点输出的是：FPoseContext
  └── FCompactPose：各骨骼的 Local Space 变换数组
  └── FBlendedCurve：曲线值（用于 Morph Target 等）
  └── FStackCustomAttributes：自定义属性

节点混合时：
  BlendTwoPoses(PoseA, PoseB, Alpha, OutPose)
    → 对每根骨骼的 FTransform 做 Lerp（位置/缩放）+ Slerp（旋转）
    → 合并曲线值（加权平均）
```

---

## 六、AnimGraph 调试技巧

```
1. 暂停并检查姿势
   PIE 中按 `（反引号）打开控制台
   输入：ShowDebug Animation
   → 显示当前 State Machine 状态、混合权重

2. AnimBP 调试视图
   在 AnimBP 编辑器中运行 PIE：
   点击 Debug Filter → 选择要调试的角色实例
   → 可以实时看到每个节点的激活状态和混合权重

3. 节点统计
   控制台：stat anim
   → 显示 AnimTask 耗时、骨骼数量、IK 计数

4. 骨骼可视化
   选中角色 → 细节面板 → Debug → Show Bones: All
```

---

## 七、延伸阅读

- 📄 [3.4 状态机基础](./04-state-machine-basics.md)
- 📄 [3.8 Linked AnimGraph](./08-linked-anim-graph.md)
- 🔗 [Animation Nodes 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-nodes-in-unreal-engine)
- 🔗 [Blend Space 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/blend-spaces-in-unreal-engine)
