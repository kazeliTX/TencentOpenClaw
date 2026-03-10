# 10.6 Control Rig IK 节点

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 30 分钟

---

## 一、Control Rig 中的 IK 节点总览

```
节点名                  骨骼链   典型用途
───────────────────────────────────────────────────────
Two Bone IK             2骨骼   手臂/腿部 IK（最常用）
FABRIK                  N骨骼   脊椎/触角等多骨骼链
Full Body IK            全身   同时控制多个末端效应器
Spline IK               N骨骼   样条曲线驱动（脊椎弯曲）
Point At                1骨骼   骨骼朝向目标（头部注视）
Look At                 1骨骼   同上，更多参数
Aim At Constraint       1骨骼   方向约束（更精确）
```

---

## 二、Two Bone IK 详解

```
两骨骼 IK（手臂：upperarm → lowerarm → hand）：

节点参数：
  Root:          upperarm_l    ← IK 链根骨骼
  IK Bone:       hand_l        ← 末端骨骼（末端效应器）
  Effector:      ctrl_hand_ik  ← IK 目标控制器
  
  Pole Vector:   ctrl_elbow_pv ← 极向量（控制肘部朝向）
  Pole Angle:    0°            ← 极向量旋转偏移
  
  Stretch Mode:  None          ← 是否允许骨骼拉伸
                 Scale Entire Chain  ← 均匀拉伸
                 Scale Root Bone     ← 只拉伸根骨骼
  
  Blend:         1.0           ← IK 权重（0=FK，1=IK）

极向量（Pole Vector）的重要性：
  决定中间关节（肘/膝）的朝向
  通常放置在肘部前方（肘部 PV = 前方 100cm）
  或膝盖前方（膝盖 PV = 后方 100cm）
```

---

## 三、FABRIK（多骨骼 IK）

```
FABRIK（Forward And Backward Reaching IK）：
  适合 3+ 骨骼的链（脊椎、触角、尾巴）
  
节点参数：
  Root:          spine_01
  End Effector:  ctrl_spine_top
  Iterations:    15           ← 求解迭代次数（越高越精确，开销越大）
  Precision:     0.001        ← 收敛精度（误差 < 1mm 停止迭代）
  
  Bone Settings Array（可选，每骨骼设置）：
    spine_01: Rotation Limit X: (-30, 30)
    spine_02: Rotation Limit X: (-20, 20)
    spine_03: Rotation Limit X: (-20, 20)

性能提示：
  FABRIK 每次迭代约 0.01ms / 骨骼
  脊椎 FABRIK（5骨骼，10迭代）≈ 0.5ms
  → LOD1+ 可将 Iterations 降到 5
```

---

## 四、Full Body IK（FBIK）

```
FBIK 同时优化整个身体所有骨骼以满足多个效应器约束：

节点配置：
  Root: pelvis（骨盆作为 FBIK 根）
  
  Effectors 列表：
    [0] ctrl_foot_l    Position: 左脚目标位置   PullWeight: 1.0
    [1] ctrl_foot_r    Position: 右脚目标位置   PullWeight: 1.0
    [2] ctrl_hand_l    Position: 左手目标位置   PullWeight: 0.8
    [3] ctrl_hand_r    Position: 右手目标位置   PullWeight: 0.8
    [4] ctrl_head      Rotation: 头部朝向       PullWeight: 0.5
  
  Solver Settings:
    Max Iterations: 20
    Solve Scale:    1.0
    
  Bone Settings（对每根骨骼设置约束）：
    pelvis: Position Enabled（骨盆可移动以平衡身体）
    spines: Rotation Stiffness: 0.5（脊椎保持相对刚硬）
    knees:  Preferred Angle: (90, 0, 0)（膝盖朝前）
```

---

## 五、延伸阅读

- 📄 [代码示例：Control Rig C++ 调用](./code/01_control_rig_cpp.cpp)
- 🔗 [Full Body IK 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/full-body-ik-in-unreal-engine)
