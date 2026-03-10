# 4.7 速度直接控制

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、直接设置速度的使用场景

```
直接设速度 vs 施加力/冲量：
  施加力/冲量：物理正确，有质量感，与其他物体交互自然
  直接设速度：精确控制，无质量感，可能破坏物理一致性

推荐用直接设速度的场景：
  ✅ 传送带（传送带表面物体强制获得传送速度）
  ✅ 拾取时初始速度（扔出去的物体获得初速）
  ✅ 受击飞（角色死亡时精确控制飞出方向）
  ✅ 初始化物理对象（给定初始速度方向）

不推荐用直接设速度的场景：
  ❌ 持续力（用 AddForce 更自然）
  ❌ 碰撞期间（会破坏碰撞响应）
  ❌ 已有约束的物体（会打破约束）
```

---

## 二、传送带实现

```cpp
UCLASS()
class AConveyorBelt : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* BeltMesh;
    UPROPERTY(EditAnywhere)    FVector BeltVelocity = FVector(200.f, 0, 0); // cm/s

    virtual void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);
        
        // 获取传送带面上的所有物理对象
        TArray<AActor*> OverlappingActors;
        BeltMesh->GetOverlappingActors(OverlappingActors);
        
        for (AActor* Actor : OverlappingActors)
        {
            if (UPrimitiveComponent* Comp =
                Actor->FindComponentByClass<UPrimitiveComponent>())
            {
                if (!Comp->IsSimulatingPhysics()) continue;
                
                // 获取传送带世界速度
                FVector WorldBeltVel = GetActorTransform().TransformVector(BeltVelocity);
                
                // 当前速度
                FVector CurrentVel = Comp->GetPhysicsLinearVelocity();
                
                // 只调整传送方向的速度（不影响垂直方向）
                FVector BeltDir = WorldBeltVel.GetSafeNormal();
                float CurrentBeltComp = FVector::DotProduct(CurrentVel, BeltDir);
                float TargetBeltComp  = WorldBeltVel.Size();
                
                // 用力推（而非直接设速度），更自然
                float DeltaV = TargetBeltComp - CurrentBeltComp;
                float Mass   = Comp->GetMass();
                Comp->AddForce(BeltDir * DeltaV * Mass / DeltaTime * 0.3f);
            }
        }
    }
};
```

---

## 三、速度钳制（最大速度限制）

```cpp
void ClampPhysicsVelocity(UPrimitiveComponent* Comp,
    float MaxLinearSpeed, float MaxAngularSpeedDeg)
{
    // 线速度钳制
    FVector Vel = Comp->GetPhysicsLinearVelocity();
    if (Vel.SizeSquared() > FMath::Square(MaxLinearSpeed))
        Comp->SetPhysicsLinearVelocity(Vel.GetSafeNormal() * MaxLinearSpeed);
    
    // 角速度钳制
    FVector AngVel = Comp->GetPhysicsAngularVelocityInDegrees();
    if (AngVel.SizeSquared() > FMath::Square(MaxAngularSpeedDeg))
        Comp->SetPhysicsAngularVelocityInDegrees(
            AngVel.GetSafeNormal() * MaxAngularSpeedDeg);
}

// 在 Tick 中调用（或 SubstepTick 更准确）
void Tick(float Dt)
{
    ClampPhysicsVelocity(Mesh, 2000.f, 3600.f);
}
```

---

## 四、延伸阅读

- 📄 [4.8 质心高级应用](./08-center-of-mass-advanced.md)
- 📄 [4.9 物理模拟事件](./09-physical-simulation-events.md)
