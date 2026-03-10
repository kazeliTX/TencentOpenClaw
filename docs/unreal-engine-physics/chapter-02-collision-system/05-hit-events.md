# 2.5 Hit Event 详解

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、Hit Event 触发条件

```
OnComponentHit 触发条件（同时满足）：
  1. CollisionEnabled = QueryAndPhysics 或 PhysicsOnly
  2. bNotifyRigidBodyCollision = true（Details → Physics → Simulation Generates Hit Events）
  3. 双方碰撞响应结果为 Block（两者都 Block 对方的通道）
  4. 相对速度超过 Bounce Threshold Velocity（默认 200 cm/s）
     低速接触（静止接触）不触发 Hit 事件！

常见误区：
  ❌ Overlap 事件不是 Hit 事件（Overlap 不阻挡）
  ❌ 静态物体相互 Block 不触发 Hit（无相对运动）
  ❌ 速度低于 Bounce Threshold → 不触发

设置 Simulation Generates Hit Events：
  蓝图：Details → Physics → Simulation Generates Hit Events = ✓
  C++：Comp->SetNotifyRigidBodyCollision(true);
```

---

## 二、Hit 事件签名

```cpp
// Component Hit 事件
UFUNCTION()
void OnHit(
    UPrimitiveComponent* HitComp,     // 我的碰撞体
    AActor*             OtherActor,   // 对方 Actor
    UPrimitiveComponent* OtherComp,   // 对方碰撞体
    FVector             NormalImpulse,// 碰撞冲量法线（kg*cm/s，施加在我身上）
    const FHitResult&   Hit           // 详细碰撞信息
);

// Actor Hit 事件（更简洁，包装 Component Hit）
UFUNCTION()
void OnActorHit(
    AActor* SelfActor,
    AActor* OtherActor,
    FVector NormalImpulse,
    const FHitResult& Hit
);

// 绑定示例（BeginPlay 中）：
Mesh->OnComponentHit.AddDynamic(this, &AMyActor::OnHit);
// 或 Actor 级别：
OnActorHit.AddDynamic(this, &AMyActor::OnActorHitCB);
```

---

## 三、NormalImpulse 的计算与应用

```
NormalImpulse（碰撞冲量）：
  方向：碰撞法线方向（从对方表面法线指向自身）
  大小：取决于两者质量、速度、弹性系数
  
  碰撞冲量公式：
    J = -(1+e) * v_rel·n / (1/m_a + 1/m_b + ...)
    
    e = Restitution（弹性系数，由 PhysicsMaterial 决定）
    v_rel = 相对速度
    n = 碰撞法线
    m_a, m_b = 两物体质量

  应用场景：
    伤害计算：damage = NormalImpulse.Size() * DamageMultiplier
    音效强度：SoundVolume = FMath::Clamp(ImpulseMag/50000.f, 0.f, 1.f)
    碰撞粒子：SpawnEmitter(scale based on ImpulseMag)

// 示例：基于碰撞冲量计算伤害
void AMyActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    float ImpulseMag = NormalImpulse.Size();
    
    // 冲量阈值，低于此不造成伤害（避免轻触触发伤害）
    const float DamageThreshold = 30000.f;
    if (ImpulseMag > DamageThreshold)
    {
        float Damage = (ImpulseMag - DamageThreshold) * 0.001f;
        UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, nullptr);
    }
    
    // 播放碰撞音效（音量随冲量线性变化）
    float Volume = FMath::GetMappedRangeValueClamped(
        FVector2D(DamageThreshold, 200000.f), FVector2D(0.2f, 1.f), ImpulseMag);
    UGameplayStatics::PlaySoundAtLocation(this, HitSound, Hit.ImpactPoint, Volume);
}
```

---

## 四、Hit Event 调试

```
常见问题排查：
  Q：为什么 Hit 事件没有触发？
  检查列表：
    ✅ CollisionEnabled = QueryAndPhysics 或 PhysicsOnly
    ✅ Simulation Generates Hit Events = true（至少一方）
    ✅ 双方对彼此的通道响应 = Block（两者都是！）
    ✅ 相对速度 > Bounce Threshold（200 cm/s 默认）
    ✅ 在 BeginPlay 中已绑定 AddDynamic

  Q：Hit 事件触发太频繁？
  原因：物体在静止接触时微小振动
  修复：
    增大 LinearDamping / AngularDamping
    增大 Sleep Threshold（让物体更快 Sleep）
    在回调中添加冷却时间：
      float LastHitTime = 0.f;
      if (GetWorld()->GetTimeSeconds() - LastHitTime < 0.1f) return;
      LastHitTime = GetWorld()->GetTimeSeconds();

  调试日志：
    void OnHit(...)
    {
        UE_LOG(LogTemp, Log, TEXT("HIT! Impulse=%.0f OtherActor=%s"),
            NormalImpulse.Size(), *OtherActor->GetName());
    }
```

---

## 五、延伸阅读

- 📄 [2.6 Overlap Event 详解](./06-overlap-events.md)
- 📄 [代码示例：Hit & Overlap 事件处理](./code/02_hit_overlap_events.cpp)
