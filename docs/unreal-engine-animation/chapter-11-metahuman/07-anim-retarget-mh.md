# 11.7 MetaHuman 动画重定向

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 20 分钟

---

## 一、官方 MetaHuman IK Rig

```
MetaHuman 提供官方预配置的 IK Rig：

位置：
  MetaHumans/Common/Body/
    IKRig_MetaHuman_Body         ← 身体 IK Rig（标准）
    IKRig_MetaHuman_Body_Female  ← 女性身体
    
预配置内容：
  根骨骼：root
  IK Goals（关键末端）：
    IKGoal_Foot_L / _R
    IKGoal_Hand_L / _R
    IKGoal_Head
    IKGoal_Pelvis
  
  Retarget Chains（骨骼链）：
    Root, Spine, LeftArm, RightArm, LeftLeg, RightLeg, Head, LeftClavicle, RightClavicle

使用方式：
  创建 IK Retargeter 时，Target 直接选 IKRig_MetaHuman_Body
  不需要自己配置目标 IK Rig
```

---

## 二、常见重定向来源

```
来源                     需要自建 IK Rig？   兼容性
───────────────────────────────────────────────────────
UE5 Mannequin           否（官方提供）      直接兼容（同骨骼）
UE4 Mannequin           需要               基本兼容（需对齐 T-Pose）
Mixamo（Y-Bot）         需要               良好（规范骨骼）
Rokoko 动捕数据         需要               需要骨骼映射
自定义角色              需要               取决于骨骼质量

UE5 Mannequin 与 MetaHuman 完全兼容：
  两者使用相同的骨骼命名规范
  不需要 IK Retargeter，动画可直接复用！
  → Marketplace 上的 UE5 动画包可直接用于 MetaHuman
```

---

## 三、重定向质量优化

```
常见重定向问题及修复：

问题 1：脚部滑动（脚步不贴地）
  原因：骨骼比例不同导致步幅不匹配
  修复：使用 Distance Matching（第六章）
       + Stride Warping（步幅自适应）

问题 2：手部错位（握武器时手部位置不对）
  原因：武器 Socket 位置基于源骨骼，不匹配 MetaHuman 手骨骼
  修复：重新校准武器 Socket 位置
       或使用 Control Rig 手部 IK 对齐

问题 3：脊椎弯曲过度或不足
  原因：骨骼链长度差异
  修复：在 IK Retargeter → Chain Settings 中调整：
    Blend to Source: 0.5（一半跟随源动画，一半由 IK 修正）

问题 4：肩膀穿插
  原因：手臂骨骼方向不同
  修复：在 IK Retargeter 中对齐 T-Pose，确保肩部骨骼对齐
```

---

## 四、延伸阅读

- 🔗 [IK Retargeter 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-animation-retargeting-in-unreal-engine)
- 🔗 [MetaHuman 动画重定向官方教程](https://dev.epicgames.com/documentation/en-us/metahuman/retargeting-animations-to-metahumans)
