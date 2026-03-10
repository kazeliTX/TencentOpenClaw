# 10.5 浮力系统

> **难度**：⭐⭐⭐⭐☆

## 一、BuoyancyComponent（Water Plugin 内置）

```cpp
// UBuoyancyComponent：与 Water Plugin 深度集成
// 自动从水体获取水面高度并施加阿基米德浮力

UPROPERTY(VisibleAnywhere) UBuoyancyComponent* Buoyancy;

// 配置（Details 面板）：
//   Pontoons（浮点）：在网格体上添加多个浮点位置
//     每个浮点独立计算是否在水面以下
//     → 多浮点 = 更自然的倾斜/翻滚
//   BuoyancyData：
//     BuoyancyCoefficient：浮力系数（1.0 = 正好漂浮）
//     BuoyancyDamp：浮力阻尼
//     BuoyancyDampForSwimming：游泳阻尼

// 船只典型配置：
//   4个浮点（前左/前右/后左/后右）
//   BuoyancyCoefficient = 1.05（略微上浮）
//   BuoyancyDamp = 3.0（波浪中不剧烈晃动）
```

## 二、手动浮力（不依赖 Water Plugin）

```cpp
// 简单浮力：仅用水面 Z 轴高度计算
void ApplySimpleBuoyancy(UPrimitiveComponent* Comp,
    float WaterSurfaceZ, float BuoyancyForce)
{
    FVector Loc = Comp->GetComponentLocation();
    float Depth = WaterSurfaceZ - Loc.Z;
    if (Depth < 0.f) return; // 在水面以上

    // 阿基米德浮力（F = ρVg）
    float Force = FMath::Clamp(Depth * BuoyancyForce, 0.f, BuoyancyForce * 2.f);
    Comp->AddForce(FVector(0, 0, Force));

    // 水阻（速度与水面法线方向的分量）
    FVector Vel = Comp->GetPhysicsLinearVelocity();
    float DragCoeff = 0.5f;
    Comp->AddForce(-Vel * DragCoeff * Comp->GetMass());
}
```
