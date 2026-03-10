# 8.2 物理动画 Profile 配置

> **难度**：⭐⭐⭐⭐☆

## 一、在 PhAT 中创建 Physical Animation Profile

```
步骤：
  1. 打开 PhysicsAsset（PhAT 编辑器）
  2. Physics Asset Editor → Physical Animations（下拉菜单）
  3. New Physical Animation Profile → 命名（如 "HitReact"）
  4. 在 Skeleton Tree 中选中需要配置的 Body
  5. Details → Physical Animation Settings：
     IsLocalSimulation：通常 true（局部弹簧）
     OrientationStrength：200~2000（越大越快追随动画旋转）
     AngularVelocityStrength：20~200（阻尼，减少振荡）
     PositionStrength：100~1000（位置追随，IsLocal=false 时生效）
     VelocityStrength：10~100（速度阻尼）
  6. Assign to Selected Bodies → 保存

常见 Profile 配置：
  HitReact（命中反应）：
    OrientationStrength = 500
    AngularVelocityStrength = 50
    → 中等追随，命中时明显摆动后缓慢回位

  Cinematic（过场精确）：
    OrientationStrength = 5000
    AngularVelocityStrength = 500
    → 强追随，几乎贴合动画，几乎看不出物理
    
  Ragdoll_Blend（布娃娃过渡）：
    OrientationStrength = 50
    AngularVelocityStrength = 10
    → 弱追随，大部分是物理，少量动画引导
```
