# 8.1 物理动画组件总览

> **难度**：⭐⭐⭐⭐☆

## 一、UPhysicalAnimationComponent 原理

```
物理动画 = 动画驱动的物理约束
  AnimBP 输出姿势 → 作为每个骨骼的"目标位置"
  PhysicalAnimation Component 向各骨骼施加弹簧力
  → 让骨骼在物理模拟的同时追随动画姿势

效果：
  角色播放走路动画（骨骼随动画运动）
  同时受到命中力/碰撞力（骨骼在物理下摆动）
  → 两者混合：既有正常动画，又有物理反应感

vs 普通 PhysicsBlendWeight：
  PhysicsBlendWeight：硬切（0=动画，1=物理）
  PhysicalAnimation：软混合（弹簧力让物理追随动画，可调强度）
  → PhysicalAnimation 更流畅，更适合命中反应
```

## 二、基本配置

```cpp
// 在角色初始化时配置物理动画
UPROPERTY(VisibleAnywhere) UPhysicalAnimationComponent* PhysAnim;

void BeginPlay()
{
    PhysAnim->SetSkeletalMeshComponent(GetMesh());
    
    // 对 spine_01 以下所有骨骼应用 Profile "HitReact"
    PhysAnim->ApplyPhysicalAnimationProfileBelow(
        FName("spine_01"),
        FName("HitReact"),
        true,   // bIncludeSelf
        true);  // bClearNotFound
    
    // 开启物理模拟
    GetMesh()->SetAllBodiesBelowSimulatePhysics(
        FName("spine_01"), true, true);
}
```

## 三、核心 API

```
ApplyPhysicalAnimationProfileBelow(BoneName, ProfileName, bIncludeSelf, bClearNotFound)
ApplyPhysicalAnimationSettings(BoneName, FPhysicalAnimationData)
SetStrengthMultiplier(StrengthMultiplier, BoneName)  ← 全局/逐骨骼调整强度

FPhysicalAnimationData 参数：
  bIsLocalSimulation：局部还是世界空间弹簧
  OrientationStrength：旋转追随强度
  AngularVelocityStrength：角速度阻尼
  PositionStrength：位置追随强度（只在 bIsLocalSimulation=false 时生效）
  VelocityStrength：速度阻尼
  MaxLinearForce：最大线性力限制
  MaxAngularForce：最大角力矩限制
```
