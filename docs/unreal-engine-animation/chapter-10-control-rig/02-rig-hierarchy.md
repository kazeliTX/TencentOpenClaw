# 10.2 Rig 层级：骨骼、控制器与空间

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、Rig Hierarchy 元素类型

```
Rig Hierarchy 包含四种类型的元素：

1. Bone（骨骼）
   来自 Skeletal Mesh 的真实骨骼
   名称与 Mesh 骨骼完全对应
   Rig Graph 最终目标：计算每根骨骼的变换

2. Control（控制器）
   动画师/运行时用于驱动骨骼的可交互对象
   有可见的 Gizmo（在 Sequencer 中可选中 K 帧）
   类型：Float、Vector、Rotator、Transform、Bool 等
   最终通过 Rig Graph 将 Control 的值传递给 Bone

3. Null（空节点）
   不可见，用于组织层级（相当于 Maya 的 Group）
   为 Control 提供父空间（如：在骨骼局部空间中定义控制器）

4. Space（空间）
   自定义坐标空间
   用于在不同骨骼/控制器之间转换变换
```

---

## 二、控制器与骨骼的对应关系

```
常见 Rig 设计模式：

A. 直接驱动（1 Control → 1 Bone）
   最简单，控制器直接映射到骨骼
   适合：FK 控制器（手臂、手指关节）
   
   Rig Graph：
     Get Control Transform (ctrl_upperarm)
     → Set Bone Transform (upperarm_l)

B. IK 驱动（1 Control → N Bones）
   控制器是 IK 目标，通过 IK 算法驱动骨骼链
   适合：脚部 IK、手部 IK
   
   Rig Graph：
     Get Control Transform (ctrl_foot_ik)
     → Two Bone IK (thigh, calf, foot)
     → Set Bone Transforms

C. 程序化驱动（N Controls → 程序 → N Bones）
   多个控制器输入，程序计算骨骼变换
   适合：脊椎弯曲、呼吸动画、复杂 IK
```

---

## 三、空间与坐标系

```
Control Rig 中的空间系统：

GlobalSpace（全局/世界空间）
  所有变换相对于世界原点
  适合：IK 目标（脚步固定在地面）

LocalSpace（局部空间）
  变换相对于父节点
  适合：FK 控制器（相对关节旋转）

ComponentSpace（组件空间）
  相对于 Skeletal Mesh 组件的根
  AnimGraph 中默认空间

在 Rig Graph 中切换空间：
  Get Bone Transform (Global) → 得到全局空间骨骼位置
  Get Control Transform (Local) → 得到局部空间控制器值
  
  Rig Graph 节点通常有 Space 参数：
  Space: Local / Global / Component
```

---

## 四、控制器初始化（Setup Graph）

```
Control Rig 有两个图：

1. Setup Graph（初始化，只在创建时执行一次）
   设置控制器的初始位置、旋转
   通常将控制器放置到对应骨骼位置
   
   // 典型 Setup：
   InitialBonePos = Get Bone Transform (hand_l, Global)
   Set Control Transform (ctrl_hand_l, InitialBonePos, Global)

2. Solve Graph（每帧执行）
   实际的求解逻辑
   从控制器读取值 → 计算骨骼变换 → 设置骨骼
```

---

## 五、延伸阅读

- 📄 [10.3 Rig Graph 编程](./03-rig-graph.md)
- 🔗 [Rig Hierarchy 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/rig-hierarchy-in-unreal-engine)
