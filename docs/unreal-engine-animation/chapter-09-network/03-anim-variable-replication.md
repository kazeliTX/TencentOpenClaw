# 9.3 AnimInstance 变量复制

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、AnimInstance 不支持直接 Replicate

```
重要限制：
  UAnimInstance 本身不支持 UPROPERTY(Replicated)
  AnimInstance 只存在于客户端（每个 Client 有自己的副本）
  服务端没有 AnimInstance（服务端不渲染动画）

正确做法：
  在 Character（或 PlayerState）中声明 Replicated 属性
  AnimInstance 读取 Character 的 Replicated 属性
  
数据流：
  Server 逻辑 → Character.ReplicatedVar
                       ↓ 网络复制
  Client Character.ReplicatedVar → AnimInstance 读取 → 驱动 AnimGraph
```

---

## 二、标准变量同步模式

```cpp
// MyCharacter.h
UCLASS()
class AMyCharacter : public ACharacter
{
    // ── 需要同步的动画驱动变量 ──────────────────────
    UPROPERTY(Replicated, BlueprintReadOnly)
    float   ReplicatedSpeed      = 0.f;

    UPROPERTY(Replicated, BlueprintReadOnly)
    bool    bIsInCombat          = false;

    UPROPERTY(Replicated, BlueprintReadOnly)
    uint8   LocomotionState      = 0;    // 0=Idle,1=Walk,2=Run,3=Sprint

    UPROPERTY(Replicated, BlueprintReadOnly)
    FVector_NetQuantize ReplicatedVelocity;  // 压缩的速度向量（精度 1cm）

    // ── 服务端设置，自动同步到所有客户端 ─────────────
    void SetCombatState(bool bInCombat)
    {
        if (HasAuthority())  // 只有 Server 可以设置
            bIsInCombat = bInCombat;
    }
};

// MyAnimInstance.h
UCLASS()
class UMyAnimInstance : public UAnimInstance
{
    UPROPERTY(BlueprintReadOnly) float   Speed;
    UPROPERTY(BlueprintReadOnly) bool    bInCombat;
    UPROPERTY(BlueprintReadOnly) uint8   LocoState;

    virtual void NativeUpdateAnimation(float DeltaSeconds) override
    {
        AMyCharacter* C = Cast<AMyCharacter>(TryGetPawnOwner());
        if (!C) return;
        // 直接读取 Character 上的 Replicated 变量
        Speed    = C->ReplicatedVelocity.Size2D();
        bInCombat = C->bIsInCombat;
        LocoState = C->LocomotionState;
    }
};
```

---

## 三、本地预测 vs 同步数据的处理

```cpp
// 对于本地控制的角色，直接用本地数据（无延迟，更流畅）
// 对于 SimulatedProxy，用 Replicated 数据
virtual void NativeUpdateAnimation(float DeltaSeconds) override
{
    AMyCharacter* C = Cast<AMyCharacter>(TryGetPawnOwner());
    if (!C) return;

    if (C->IsLocallyControlled())
    {
        // 本地控制：直接读取，最准确
        Speed        = C->GetCharacterMovement()->Velocity.Size2D();
        bIsInAir     = C->GetCharacterMovement()->IsFalling();
        AimPitch     = C->GetController() ?
                       C->GetController()->GetControlRotation().Pitch : 0.f;
    }
    else
    {
        // 远程角色：读取网络同步数据
        Speed        = C->ReplicatedVelocity.Size2D();
        bIsInAir     = C->GetCharacterMovement()->IsFalling(); // CMC 自动同步
        AimPitch     = (float)C->RemoteViewPitch / 255.f * 360.f;
        if (AimPitch > 180.f) AimPitch -= 360.f;
    }
}
```

---

## 四、带宽优化：压缩类型

```
用专用压缩类型减少带宽：

类型                      字节数   精度
──────────────────────────────────────────
FVector                   12 B     精确
FVector_NetQuantize       12 B     1cm 精度（自动压缩）
FVector_NetQuantize10     6 B      10cm 精度
FVector_NetQuantize100    4 B      1m 精度（人群足够）
FRotator                  12 B     精确
uint8（RemoteViewPitch）  1 B      1.4° 精度

推荐：
  速度方向用 FVector_NetQuantize（1cm 足够）
  位置用 CMC 内置（已优化）
  角度用 uint8（RemoteViewPitch 模式）
```

---

## 五、延伸阅读

- 📄 [代码示例：完整 AnimInstance](./code/01_replicated_anim_instance.cpp)
- 🔗 [属性复制文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/replicated-properties-in-unreal-engine)
