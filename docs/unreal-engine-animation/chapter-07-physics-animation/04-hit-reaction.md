# 7.4 受击反应（Hit Reaction）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、受击反应技术对比

```
方案对比：

方案 A：Additive 动画（轻量级）
  预制不同方向的受击动画（前后左右）
  根据受击方向叠加到基础动画上
  开销低，效果可预测
  缺点：只有固定几个方向，不够动态

方案 B：Physical Animation（物理驱动，推荐）
  Physical Animation Component 提供物理约束
  受击时降低约束强度，施加冲量
  骨骼自然物理晃动，然后被拉回动画姿势
  效果真实，方向完全动态
  开销中等

方案 C：局部布娃娃（重量级）
  受击部位切换为完全物理模拟
  全程布娃娃效果但只影响局部骨骼链
  效果最真实，开销最高
  适合：被击飞、被爆炸冲击
```

---

## 二、基于 Physical Animation 的受击反应

```cpp
// 受击反应系统（依赖 Physical Animation Component）
void UHitReactionComponent::ReactToHit(
    FVector WorldHitLocation,
    FVector HitImpulse,
    float   ImpulseStrength)
{
    USkeletalMeshComponent* Mesh = GetOwnerMesh();
    if (!Mesh || !PhysAnim) return;

    // 1. 找到最近受击骨骼
    FName HitBone = Mesh->FindClosestBone(WorldHitLocation);

    // 2. 确保骨骼在物理模拟中（需要预先配置 Physics Asset）
    Mesh->SetAllBodiesBelowSimulatePhysics(HitBone, true, true);

    // 3. 降低物理混合权重（让骨骼自由响应冲量）
    Mesh->SetAllBodiesBelowPhysicsBlendWeight(
        HitBone, HitReactionPhysicsWeight, false, true);

    // 4. 在受击位置施加冲量
    FVector NormalizedImpulse = HitImpulse.GetSafeNormal() * ImpulseStrength;
    Mesh->AddImpulseAtLocation(NormalizedImpulse, WorldHitLocation);

    // 5. 设定恢复定时器
    float RecoveryTime = FMath::GetMappedRangeValueClamped(
        FVector2D(100.f, 1000.f),   // 冲量范围
        FVector2D(0.15f, 0.5f),     // 恢复时间范围（小冲量快速恢复）
        ImpulseStrength);

    // 取消已有恢复计时，重置
    GetWorld()->GetTimerManager().ClearTimer(RecoveryTimer);
    GetWorld()->GetTimerManager().SetTimer(RecoveryTimer, [this, HitBone]()
    {
        RecoverFromHit(HitBone);
    }, RecoveryTime, false);
}

void UHitReactionComponent::RecoverFromHit(FName TopBone)
{
    USkeletalMeshComponent* Mesh = GetOwnerMesh();
    if (!Mesh) return;

    // 平滑恢复物理权重（插值恢复到 1.0）
    RecoveryAlpha = 0.0f;
    bIsRecovering = true;
    RecoveryTopBone = TopBone;
}

void UHitReactionComponent::TickComponent(float DeltaTime, ...)
{
    if (!bIsRecovering) return;

    RecoveryAlpha = FMath::Clamp(RecoveryAlpha + DeltaTime / RecoveryDuration, 0.f, 1.f);
    
    USkeletalMeshComponent* Mesh = GetOwnerMesh();
    if (Mesh)
    {
        // 逐渐恢复到跟随动画（PhysicsBlendWeight → 1.0）
        float BlendWeight = FMath::Lerp(HitReactionPhysicsWeight, 1.0f, RecoveryAlpha);
        Mesh->SetAllBodiesBelowPhysicsBlendWeight(
            RecoveryTopBone, BlendWeight, false, true);
    }

    if (RecoveryAlpha >= 1.0f)
    {
        bIsRecovering = false;
        // 可选：恢复后关闭物理（如只想要短暂受击，不需要持续物理）
        // Mesh->SetAllBodiesBelowSimulatePhysics(RecoveryTopBone, false, true);
    }
}
```

---

## 三、受击方向检测

```cpp
// 根据攻击者位置判断受击方向（用于 Additive 动画方案）
EHitDirection UHitReactionComponent::GetHitDirection(
    FVector AttackerLocation) const
{
    ACharacter* Owner = Cast<ACharacter>(GetOwner());
    if (!Owner) return EHitDirection::Front;

    FVector ToAttacker = (AttackerLocation - Owner->GetActorLocation()).GetSafeNormal2D();
    FVector Forward    = Owner->GetActorForwardVector();
    FVector Right      = Owner->GetActorRightVector();

    float ForwardDot = FVector::DotProduct(Forward, ToAttacker);
    float RightDot   = FVector::DotProduct(Right, ToAttacker);

    // 判断 8 方向（45°扇区）
    if (ForwardDot > 0.71f)  return EHitDirection::Front;
    if (ForwardDot < -0.71f) return EHitDirection::Back;
    if (RightDot > 0.71f)    return EHitDirection::Right;
    if (RightDot < -0.71f)   return EHitDirection::Left;
    return (ForwardDot > 0) ?
        (RightDot > 0 ? EHitDirection::FrontRight : EHitDirection::FrontLeft) :
        (RightDot > 0 ? EHitDirection::BackRight  : EHitDirection::BackLeft);
}
```

---

## 四、延伸阅读

- 📄 [代码示例：受击反应系统](./code/02_hit_reaction.cpp)
- 🔗 [Physical Animation 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/physical-animation-in-unreal-engine)
