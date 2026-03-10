# 4.9 物理模拟事件与状态机

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、物理状态机设计

```
物理对象通常有多个状态，每个状态行为不同：

示例：手雷
  Idle（未激活）：
    Simulate Physics = false，静止
  Flying（飞行中）：
    Simulate Physics = true，CCD = on
    每帧更新轨迹（弹道计算）
  Bouncing（弹跳）：
    OnComponentHit → 播放音效/粒子
  Exploding（爆炸）：
    FireImpulse → 销毁

状态转换：
  Idle → Flying：玩家扔出时（施加初速）
  Flying → Bouncing：Hit 事件（接触地面/墙壁）
  Bouncing → Exploding：计时器到期 或 速度过低

UCLASS()
class AGrenade : public AActor
{
    GENERATED_BODY()
    enum class EState { Idle, Flying, Bouncing, Exploding };
    EState State = EState::Idle;
    FTimerHandle ExplosionTimer;
    int32 BounceCount = 0;

    void Throw(FVector Direction, float Speed)
    {
        State = EState::Flying;
        Mesh->SetSimulatePhysics(true);
        Mesh->SetCCDEnabled(true);
        Mesh->SetPhysicsLinearVelocity(Direction * Speed);
        Mesh->OnComponentHit.AddDynamic(this, &AGrenade::OnHit);
        // 3 秒后引爆
        GetWorld()->GetTimerManager().SetTimer(
            ExplosionTimer, this, &AGrenade::Explode, 3.f, false);
    }

    UFUNCTION() void OnHit(UPrimitiveComponent* C, AActor* A,
        UPrimitiveComponent* OC, FVector NI, const FHitResult& H)
    {
        State = EState::Bouncing;
        BounceCount++;
        UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
        if (BounceCount >= 3)
        {
            // 超过3次弹跳 → 立即引爆
            GetWorld()->GetTimerManager().ClearTimer(ExplosionTimer);
            Explode();
        }
    }

    void Explode()
    {
        State = EState::Exploding;
        Mesh->SetSimulatePhysics(false);
        // 播放爆炸效果
        // 施加径向冲量（见 4.4）
        Destroy();
    }
};
```

---

## 二、OnSleep / OnWake 事件利用

```
OnComponentSleep：物体静止进入 Sleep
OnComponentWake ：物体被唤醒

// 注册 Sleep 事件
Mesh->OnComponentSleep.AddDynamic(this, &AMyActor::OnSleep);
Mesh->OnComponentWake.AddDynamic(this, &AMyActor::OnWake);

UFUNCTION() void OnSleep(UPrimitiveComponent* Comp, FName BoneName)
{
    // 物体完全静止后：关闭 Tick，节省性能
    SetActorTickEnabled(false);
    
    // 如果是堆叠积木，检查是否可以合并为 Static
    if (bCanBecomStatic)
    {
        Mesh->SetMobility(EComponentMobility::Static);
        Mesh->SetSimulatePhysics(false);
        // 大幅减少运行时物理开销
    }
}

UFUNCTION() void OnWake(UPrimitiveComponent* Comp, FName BoneName)
{
    // 物体被唤醒：重新开启 Tick
    SetActorTickEnabled(true);
}
```

---

## 三、延伸阅读

- 📄 [4.10 Kinematic 驱动与混合](./10-kinematic-driven.md)
