# 5.1 IK 数学基础与 UE 中的 IK 框架

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、正向运动学 vs 逆向运动学

```
正向运动学（FK, Forward Kinematics）：
  给定每根骨骼的角度 → 计算末端位置
  方向：关节角度 → 末端位置
  特点：计算简单，动画师直接控制每根骨骼
  
逆向运动学（IK, Inverse Kinematics）：
  给定末端目标位置 → 反推每根骨骼的角度
  方向：目标位置 → 关节角度（求解过程）
  特点：适合运行时自适应（如脚踩不平地面）
  
  问题：IK 通常没有唯一解！
  例：手臂伸向目标，肘部可以在任意方向
  → 需要约束（Pole Target）指定首选方向
```

---

## 二、UE 中的 IK 节点生态

```
UE IK 技术栈（从低级到高级）：

1. AnimGraph 内置 IK 节点（简单，AnimBP 中直接用）
   ├── Two-Bone IK        ← 双骨骼 IK（手臂/腿部）
   ├── FABRIK             ← 链式 IK（多关节）
   ├── Look At            ← 骨骼注视目标
   └── Spline IK          ← 样条 IK（脊椎/尾巴）

2. Full Body IK（FBIK，UE5 新增）
   └── 基于约束的全身 IK，支持多目标同时求解

3. IK Rig + IK Retargeter（UE5，编辑器工具）
   ├── IK Rig：定义骨骼 IK 链和约束
   └── IK Retargeter：跨骨骼动画重定向

4. Control Rig（最强大，用于制作工具和运行时）
   ├── 包含多种 IK 求解器（Spine IK, Limb IK, FullBody IK...）
   └── 可在 AnimGraph 中作为节点运行
```

---

## 三、IK 求解算法概述

### CCD（Cyclic Coordinate Descent）

```
最经典的迭代 IK 算法：

步骤（从末端向根循环）：
  1. 从链末端（效应器）开始
  2. 对每根骨骼：旋转该骨骼使末端尽量靠近目标
  3. 从末端到根重复，直到收敛或达到最大迭代次数

优点：实现简单，运行快
缺点：迭代次数多时收敛慢；关节分布不均匀时行为怪异
用途：UE 的 Two-Bone IK（优化版）
```

### FABRIK（Forward And Backward Reaching Inverse Kinematics）

```
更现代的迭代 IK 算法：

步骤：
  Forward Pass（从根到末端）：
    末端移到目标位置
    依次推动各关节保持骨骼长度
    
  Backward Pass（从末端到根）：
    根固定到原始位置
    依次拉动各关节保持骨骼长度
    
  重复直到收敛

优点：收敛快，关节分布自然，支持约束
用途：UE 的 FABRIK 节点（多关节链）
```

### 雅可比矩阵法（Jacobian）

```
基于微分的 IK 方法：

核心：计算雅可比矩阵 J（末端位置对各关节角度的偏导数）
然后求 J 的伪逆，将目标位移反推为关节角度变化

优点：处理冗余自由度（骨骼数 > 约束数），支持全身 IK
缺点：计算量大，矩阵运算
用途：UE 的 Full Body IK（底层使用此类方法）
```

---

## 四、IK 的关键概念

```
效应器（Effector）：
  IK 链的末端（如：手、脚），目标指定其位置/旋转

极向目标（Pole Target / Pole Vector）：
  指定关节弯曲方向的辅助点
  例：膝盖 Pole Target 在前方 → 膝盖向前弯
      膝盖 Pole Target 在侧方 → 膝盖向侧弯

IK 链（IK Chain）：
  参与 IK 求解的骨骼序列
  例：Two-Bone IK 链：大腿 → 小腿 → 脚

IK Alpha：
  IK 的混合权重（0.0 = 纯 FK，1.0 = 纯 IK）
  用于平滑开关 IK 效果

骨骼长度约束：
  IK 求解时保持骨骼长度不变（刚性骨骼假设）
```

---

## 五、延伸阅读

- 📄 [5.2 Two-Bone IK](./02-two-bone-ik.md)
- 📄 [5.7 Full Body IK](./07-full-body-ik.md)
- 🔗 [FABRIK 原始论文](http://www.andreasaristidou.com/publications/papers/FABRIK.pdf)
- 🔗 [IK 求解器综述（知乎）](https://zhuanlan.zhihu.com/p/381967985)
