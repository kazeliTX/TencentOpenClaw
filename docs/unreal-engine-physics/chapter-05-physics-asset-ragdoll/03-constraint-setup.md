# 5.3 PhAT 约束配置

> **难度**：⭐⭐⭐⭐☆

## 一、关节类型与角度限制

```
人形骨骼关节推荐设置：

关节              类型        Swing1  Swing2  Twist
spine→spine       球形关节    ±30°    ±20°    ±10°
neck→head         球形关节    ±45°    ±30°    ±20°
shoulder→arm      球形关节    ±90°    ±75°    ±80°
elbow             铰链关节    0       ±130°   0
wrist             球形关节    ±45°    ±50°    ±45°
hip→thigh         球形关节    ±60°    ±50°    ±30°
knee              铰链关节    0       ±140°   0
ankle             球形关节    ±50°    ±20°    ±20°

铰链关节（肘/膝）：
  Swing1 = Locked（0），Swing2 = Limited（弯曲方向），Twist = Locked（0）
  → 只允许单轴弯曲，就像真实关节
  
  膝盖需要设置 Reference Angle 偏移：
    把 Swing2 Limit Angle 设为 140°，但 Reference = -10°（轻微弯曲）
    → 防止膝盖超伸（hyperextension）
```

## 二、软限制调优

```
所有 PhAT 约束推荐开启软限制：
  Details → Angular Limits → Soft Swing/Twist Limits = true
  Stiffness：300~500（太低=果冻感，太高=弹射）
  Damping：30~60（欠阻尼会来回振荡）
  ContactDistance：5°~10°（提前激活软限制的缓冲区）

人形推荐值（骨骼较轻，不需要太强的力）：
  躯干关节：Stiffness=350, Damping=40
  四肢关节：Stiffness=250, Damping=30
  
  对比：
    机甲/重型机器人：Stiffness=1500, Damping=200（更硬）
    布偶/棉花角色：Stiffness=50, Damping=10（更软）
```

## 三、约束 Profile

```
PhAT 支持多个约束 Profile（Constraint Profiles）：
  Default Profile：布娃娃完全激活
  HitReaction Profile：部分关节限制松动（受击反应）
  Cinematic Profile：精确关节配置（过场动画）
  KinematicDefault：全部关节锁定（Kinematic 状态）

切换 Profile（C++）：
  // 切换到布娃娃 Profile
  GetMesh()->SetConstraintProfileForAll(FName("Ragdoll"), true);
  // 切换到 Kinematic Profile
  GetMesh()->SetConstraintProfileForAll(FName("KinematicDefault"), true);
  // 单个约束切换
  GetMesh()->SetConstraintProfile(FName("spine_01"), FName("HitReaction"), true);
```

## 四、延伸阅读

- 📄 [5.4 Ragdoll 激活流程](./04-ragdoll-activation.md)
- 📄 [5.10 Physics Asset Profiles](./10-physics-asset-profiles.md)
