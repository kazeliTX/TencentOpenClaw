# 10.3 Rig Graph（可视化编程）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、Rig Graph 基础

```
Rig Graph 是 Control Rig 的"大脑"——可视化节点编程：

执行流程：
  每帧：Execute → [节点链] → 输出骨骼变换

基本节点类型：

Get/Set 节点：
  Get Bone Transform      ← 读取骨骼当前变换
  Set Bone Transform      ← 写入骨骼变换（最终输出）
  Get Control Value       ← 读取控制器值（动画师 K 帧的值）
  Set Control Value       ← 设置控制器值（用于 Backwards Solve）

数学节点：
  Math / Float Ops        ← 加减乘除
  Quaternion              ← 四元数旋转运算
  Transform               ← 变换合并/分解
  Vector Math             ← 向量计算

IK 节点：
  Two Bone IK             ← 两骨骼链 IK
  FABRIK                  ← 多骨骼链 IK
  Full Body IK            ← 全身 IK
  Spline IK               ← 样条线 IK（脊椎用）

流程控制：
  Branch（条件分支）
  For Loop（循环）
  Sequence（顺序执行）
```

---

## 二、典型 FK 控制器实现

```
手臂 FK 控制器（Upper Arm → Lower Arm → Hand）：

Rig Graph 节点连接：
  
  [Get Control: ctrl_upperarm] ──(Rotation)──┐
                                              ▼
                                   [Set Bone: upperarm_l]
                                              │ (Execute)
  [Get Control: ctrl_lowerarm] ──(Rotation)──┐
                                              ▼
                                   [Set Bone: lowerarm_l]
                                              │ (Execute)
  [Get Control: ctrl_hand_l]   ──(Rotation)──┐
                                              ▼
                                   [Set Bone: hand_l]

Space 说明：
  FK 控制器用 Local Space（相对父骨骼旋转）
  节点参数：Set Bone Transform → Space: Local
```

---

## 三、函数（Function）与模块化

```
Rig Graph 支持自定义函数（类似蓝图 Function）：

创建函数：
  Rig Graph → 右键 → Create Function
  命名：SolveArmIK

函数参数：
  Input:  FRigElementKey IKTarget
          FRigElementKey RootBone
  Output: （修改骨骼变换，无返回值）

调用函数：
  在主 Solve Graph 中调用
  → 复用代码，保持主图清晰

Control Rig 模块（UE5.3+）：
  将常用功能打包为 Rig Module
  可在多个 Control Rig 中复用
  类似 AnimBP 的 Linked AnimGraph
```

---

## 四、变量与参数传递

```
Control Rig 变量类型：
  Local Variable：只在当前图内有效
  Member Variable：整个 Rig 共享（类似类成员）
  Input / Output Variable：与外部（AnimBP）通信

与 AnimBP 通信：
  Control Rig 节点（在 AnimGraph 中）→
    Input: 从 AnimBP 传入值（如 IK 目标位置）
    Output: 从 Control Rig 读取值（如 IK 求解后的位置）

  // 在 AnimBP EventGraph 中：
  SetControlRigInput("FootIKTarget", FootTargetLocation);
  
  // 在 Rig Graph 中：
  GetVariable("FootIKTarget") → [Two Bone IK] → Set Bone Transform
```

---

## 五、延伸阅读

- 📄 [10.4 Forward Solve](./04-forward-solve.md)
- 🔗 [Rig Graph 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/rig-graph-in-unreal-engine)
