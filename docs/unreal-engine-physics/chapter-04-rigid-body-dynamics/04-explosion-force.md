# 4.4 爆炸力场（RadialForce）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、URadialForceComponent

```
URadialForceComponent：UE 内置的径向力组件
  从中心向外施加力/冲量
  自动处理范围衰减

参数：
  Radius：力场半径（cm）
  ForceStrength：力的大小（AddForce 模式）
  ImpulseStrength：冲量大小（单次爆炸模式）
  Falloff：衰减类型
    RIF_Constant：范围内均匀力（不衰减）
    RIF_Linear：线性衰减（距离越远越小）
  bImpulseVelChange：true=速度变化（质量无关）
  bIgnoreOwningActor：忽略自身（避免自爆）

使用方式：
  持续力场（风扇/吸力）：
    RadialForce->FireImpulse() — 每帧调用（性能差）
    → 更好：在 Tick 中手动对范围内对象施加力
  
  一次性爆炸：
    RadialForce->FireImpulse() — 仅调用一次
    → 瞬间对范围内所有物理对象施加冲量
```

---

## 二、自定义爆炸系统

```cpp
// 爆炸函数（更灵活的自定义实现）
void ApplyExplosionImpulse(
    UWorld* World,
    FVector Center,
    float Radius,
    float BaseImpulse,
    float VerticalBoost = 0.3f)   // 向上分量（让物体飞起来）
{
    // 获取范围内的所有物理 Component
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    World->OverlapMultiByObjectType(
        Overlaps, Center, FQuat::Identity,
        FCollisionObjectQueryParams(ECC_PhysicsBody),
        FCollisionShape::MakeSphere(Radius), Params);
    
    TSet<UPrimitiveComponent*> AffectedComps;
    for (const FOverlapResult& R : Overlaps)
    {
        UPrimitiveComponent* Comp = R.Component.Get();
        if (!Comp || !Comp->IsSimulatingPhysics()) continue;
        if (AffectedComps.Contains(Comp)) continue;
        AffectedComps.Add(Comp);
        
        FVector CompCenter = Comp->GetComponentLocation();
        float Dist = FVector::Distance(Center, CompCenter);
        if (Dist < 1.f) Dist = 1.f;
        
        // 线性衰减
        float FallOff = FMath::Max(0.f, 1.f - Dist / Radius);
        float ImpulseMag = BaseImpulse * FallOff;
        
        // 方向：从爆炸中心朝外 + 向上分量
        FVector Dir = (CompCenter - Center).GetSafeNormal();
        Dir.Z += VerticalBoost;
        Dir.Normalize();
        
        Comp->AddImpulseAtLocation(Dir * ImpulseMag, Center);
    }
}
```

---

## 三、爆炸力衰减模型

```
常见衰减模型：

线性衰减：  F(r) = F0 * (1 - r/R)      最常用，直觉明显
平方衰减：  F(r) = F0 / (r/R)²         符合真实物理（平方反比）
指数衰减：  F(r) = F0 * e^(-k*r/R)     柔和衰减
阶梯衰减：  F(r) = F0（r < R1），0（r ≥ R1）  冲击波效果

UE 中的实现选择：
  RIF_Constant = 阶梯（范围内均匀）
  RIF_Linear   = 线性

自定义平方衰减：
  float FallOff = FMath::Clamp(
      1.f / FMath::Square(Dist / Radius + 0.1f), 0.f, 1.f);

爆炸内部区域（直接伤害区）：
  if (Dist < InnerRadius)
      FallOff = 1.0f;  // 内圈不衰减（全力）
  else
      FallOff = 1.f - (Dist - InnerRadius) / (Radius - InnerRadius);
```

---

## 四、破片系统

```
爆炸产生破片（Chaos Fracture 详见第七章）：
  爆炸 → 触发几何体集合（GeometryCollection）的裂碎
  同时对碎片施加径向冲量

快速模拟破片（不需要完整 Chaos Fracture）：
  1. 准备多个预制碎片 Mesh（预隐藏）
  2. 爆炸时激活碎片，施加随机冲量

void SpawnDebrisExplosion(FVector Center, int32 NumPieces)
{
    for (int32 i = 0; i < NumPieces; ++i)
    {
        // 生成碎片 Actor
        FVector SpawnLoc = Center + FMath::VRand() * 50.f;
        ADebrisActor* Debris = GetWorld()->SpawnActor<ADebrisActor>(
            DebrisClass, SpawnLoc, FRotator::ZeroRotator);
        
        if (Debris)
        {
            // 随机飞散冲量
            FVector Dir = FMath::VRand();
            Dir.Z = FMath::Abs(Dir.Z);  // 确保向上
            float Mag = FMath::RandRange(20000.f, 80000.f);
            Debris->GetMesh()->AddImpulse(Dir * Mag, NAME_None, false);
            
            // 随机旋转
            FVector AngImpulse = FMath::VRand() * FMath::RandRange(100.f, 500.f);
            Debris->GetMesh()->AddAngularImpulseInDegrees(AngImpulse);
            
            // 延迟销毁
            Debris->SetLifeSpan(FMath::RandRange(3.f, 8.f));
        }
    }
}
```

---

## 五、延伸阅读

- 📄 [4.5 重力变体与浮力](./05-gravity-buoyancy.md)
- 📄 [代码示例：爆炸系统](./code/02_explosion_system.cpp)
- 🔗 [Radial Force Component](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Components/URadialForceComponent)
