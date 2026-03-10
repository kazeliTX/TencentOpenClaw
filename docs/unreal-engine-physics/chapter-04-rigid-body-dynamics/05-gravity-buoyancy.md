# 4.5 重力变体与浮力

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、每物体重力缩放

```
每个 FBodyInstance 可独立设置重力缩放：
  BI->GravityScale = 0.5f;   // 半重力（缓慢飘落）
  BI->GravityScale = 0.0f;   // 零重力（漂浮）
  BI->GravityScale = 2.0f;   // 双倍重力（快速下落）
  BI->GravityScale = -1.0f;  // 反重力（向上飘）

C++ 设置：
  Mesh->GetBodyInstance()->GravityScale = 0.3f;

蓝图：Details → Physics → Gravity Scale

应用场景：
  气球（GravityScale = -0.5）→ 向上漂，但仍受水平力影响
  羽毛（GravityScale = 0.1）→ 缓慢飘落
  铅球（GravityScale = 1.5）→ 重型感
  太空舱内（GravityScale = 0）→ 失重漂浮
```

---

## 二、浮力实现

```
UE 没有内置浮力系统，需要手动实现：

浮力原理（阿基米德定律）：
  F_buoyancy = ρ_fluid * g * V_submerged
  ρ_fluid = 流体密度（水：1.0 g/cm³）
  g       = 重力加速度（980 cm/s²）
  V_submerged = 浸入流体的体积

简化实现（基于中心点深度）：
void ApplyBuoyancy(UPrimitiveComponent* Comp, float WaterSurfaceZ)
{
    if (!Comp->IsSimulatingPhysics()) return;
    
    FVector Loc = Comp->GetComponentLocation();
    float Depth = WaterSurfaceZ - Loc.Z;  // 浸水深度（cm）
    
    if (Depth <= 0.f) return;  // 完全在水面以上
    
    // 估算浸入比例（简化为深度/物体高度）
    float ObjectHalfHeight = 50.f;  // 物体半高
    float SubmergedRatio = FMath::Clamp(Depth / (2.f * ObjectHalfHeight), 0.f, 1.f);
    
    // 浮力 = 排开水的重力（F = ρ*g*V）
    float Mass  = Comp->GetMass();
    float BuoyancyForce = Mass * 980.f * SubmergedRatio * 1.2f; // 1.2=浮力系数
    
    Comp->AddForce(FVector(0, 0, BuoyancyForce));
    
    // 水阻（减缓水中运动）
    FVector Vel = Comp->GetPhysicsLinearVelocity();
    float WaterDrag = 5.f * SubmergedRatio;
    Comp->AddForce(-Vel * WaterDrag * Mass);
}

// 在 Tick 或 SubstepTick 中调用
void Tick(float Dt)
{
    for (UPrimitiveComponent* Comp : UnderwaterObjects)
        ApplyBuoyancy(Comp, WaterLevel);
}
```

---

## 三、APhysicsVolume 重力区域

```
继承 APhysicsVolume 实现局部重力区域：

UCLASS()
class ACustomGravityVolume : public APhysicsVolume
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) float CustomGravityZ = -200.f;
    UPROPERTY(EditAnywhere) bool bEnableWaterDrag = true;

    float GetGravityZ() const override
    {
        return CustomGravityZ;
    }

    // 进入区域时设置水阻
    void ActorEnteredVolume(AActor* Other) override
    {
        Super::ActorEnteredVolume(Other);
        if (!bEnableWaterDrag) return;
        
        if (UPrimitiveComponent* PC = Other->FindComponentByClass<UPrimitiveComponent>())
        {
            if (PC->IsSimulatingPhysics())
            {
                SavedLinearDamping[Other] = PC->GetLinearDamping();
                PC->SetLinearDamping(5.0f);  // 水阻
                PC->SetAngularDamping(3.0f);
            }
        }
    }

    void ActorLeavingVolume(AActor* Other) override
    {
        Super::ActorLeavingVolume(Other);
        // 恢复原始阻尼
        if (float* Saved = SavedLinearDamping.Find(Other))
        {
            if (UPrimitiveComponent* PC = Other->FindComponentByClass<UPrimitiveComponent>())
                PC->SetLinearDamping(*Saved);
            SavedLinearDamping.Remove(Other);
        }
    }

    TMap<AActor*, float> SavedLinearDamping;
};
```

---

## 四、延伸阅读

- 📄 [4.6 点施力详解](./06-force-at-location.md)
- 🔗 [Physics Volumes](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-volumes-in-unreal-engine)
