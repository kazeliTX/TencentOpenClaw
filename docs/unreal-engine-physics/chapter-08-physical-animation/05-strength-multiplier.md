# 8.5 Strength 参数详解

> **难度**：⭐⭐⭐⭐☆

## 一、SetStrengthMultiplier

```
PhysAnim->SetStrengthMultiplier(Multiplier, BoneName);

Multiplier 值的含义：
  0.0   ：关闭物理动画追随（纯物理自由运动）
  0.1~0.3：弱追随（角色被打倒时保留少量动画引导）
  0.5~0.8：中等追随（命中反应标准值）
  1.0   ：正常追随（默认）
  2.0+  ：超强追随（角色在极端速度下仍精确追随动画）

运行时动态调整（按骨骼区域）：
  // 命中头部 → 头部及以上弱追随，其余正常
  PhysAnim->SetStrengthMultiplier(0.2f, FName("head"));
  PhysAnim->SetStrengthMultiplier(0.2f, FName("neck_01"));
  PhysAnim->SetStrengthMultiplier(1.0f, FName("spine_03")); // 躯干正常

  // 落水时全身弱追随（浮在水中动画感减弱）
  PhysAnim->SetStrengthMultiplier(0.1f, NAME_None); // NAME_None = 全骨骼
  
OrientationStrength vs SetStrengthMultiplier 的区别：
  OrientationStrength：Profile 中的基础值（静态配置）
  SetStrengthMultiplier：运行时乘数（动态调整）
  
  实际追随力 = OrientationStrength × SetStrengthMultiplier
```
