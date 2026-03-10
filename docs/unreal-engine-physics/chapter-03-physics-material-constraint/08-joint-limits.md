# 3.8 约束限制（Angular/Linear Limits）

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 35 分钟

---

## 一、线性限制（Linear Limits）

```
Linear Limit（线性限制）：
  限制两体之间在某轴上的相对位移范围
  
  Linear X/Y/Z Motion = Limited → 启用该轴的限制
  Linear Limit Size = N cm（双向，即 ±N cm）
  
  软限制（Soft Constraint）：
    启用后：超出 Limit 时施加弹性恢复力（非立即锁死）
    Soft Linear Limit Stiffness：超出后的弹簧刚度
    Soft Linear Limit Damping：超出后的阻尼
    越接近 Limit → 力越大（类似弹簧被压缩）
  
  硬限制：
    超出 Limit 立即停止（硬接触）
    可能导致抖动（无缓冲）
    
  推荐：大多数关节用软限制 + 适当阻尼

接触距离（Contact Distance）：
  在距离 Limit 多远时开始施加约束力
  越大 → 提前约束，更平滑（但可视范围更大）
  越小 → 临近 Limit 才施加力，更精确
  默认：0.0（到达 Limit 才激活）
```

---

## 二、角度限制（Angular Limits）

```
Swing Limits（摆动限制）：
  控制 Swing 1（Y平面摆动）和 Swing 2（Z平面摆动）的角度范围
  Swing Limit Angle：[0°, 180°]
  0° = 完全锁定（Locked），180° = 完全自由（Free）
  典型：肩关节 Swing 1/2 = 70° 左右

Twist Limit（扭转限制）：
  控制绕 X 轴旋转的范围
  Twist Limit Angle：±N°（双向对称）
  典型：肘关节 Twist = ±90°

软角度限制：
  Soft Swing / Soft Twist Limit：启用软限制
  Swing/Twist Limit Stiffness：超出时的恢复刚度
  Swing/Twist Limit Damping：超出时的阻尼
  
关节角度限制设计（人形角色 Ragdoll）：
  ─────────────────────────────────────────────────────────
  关节          Swing1  Swing2  Twist   说明
  ─────────────────────────────────────────────────────────
  neck_01       30°     30°     30°     颈椎（有限活动）
  spine_01      20°     20°     30°     腰椎（有限弯曲）
  spine_03      20°     20°     20°     胸椎
  shoulder_l/r  70°     70°     80°     肩关节（球窝）
  upperarm_l/r  10°     70°     90°     肘关节（有限摆动）
  lowerarm_l/r  0°      0°      90°     前臂（只扭转）
  thigh_l/r     70°     45°     60°     髋关节（球窝）
  calf_l/r      0°      80°     0°      膝关节（单向弯曲）
  foot_l/r      30°     20°     10°     踝关节（有限）
  ─────────────────────────────────────────────────────────
```

---

## 三、软限制调优

```
软限制参数调优策略：

"门"的软限制调优：
  门开到 90°时触碰墙壁
  如果是硬限制 → 门撞到 90° 时反弹/抖动
  
  软限制配置（平滑停止）：
    Swing Limit Angle = 90°
    Soft Swing Limit = true
    Swing Limit Stiffness = 200（越大越快弹回，太大会振荡）
    Swing Limit Damping = 20（抑制振荡）
  
  效果：门快到 90° 时感觉有阻力，平滑停下

"手臂" Ragdoll 软限制调优：
  希望手臂有生理感，达到极限时平滑停止
  
  elbow（肘关节）调优：
    Swing Limit = 130°（向前弯曲）
    Soft Swing Limit = true
    Stiffness = 100
    Damping = 50（强阻尼，到极限很快停下）
  
  "橡胶感" 调优（低刚度，高阻尼）：
    Stiffness = 50（轻柔弹回）
    Damping = 100（快速静止，不振荡）
  
  "弹性感" 调优（高刚度，低阻尼）：
    Stiffness = 500（强力弹回）
    Damping = 20（轻阻尼，会振荡几次）
```

---

## 四、延伸阅读

- 📄 [3.9 约束驱动器（Motor）](./09-constraint-motor.md)
- 🔗 [Constraint User Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-constraints-user-guide-in-unreal-engine)
