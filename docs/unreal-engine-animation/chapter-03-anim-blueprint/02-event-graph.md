# 3.2 Event Graph 与变量更新

> **难度**：⭐⭐☆☆☆ | **阅读时间**：约 20 分钟

---

## 一、Event Graph 的职责

Event Graph 运行在 **Game Thread**，是 AnimBP 的"数据收集层"：

```
Event Graph 只做一件事：
  从 Gameplay 世界收集数据 → 写入 AnimBP 变量 → 供 AnimGraph 读取

正确的流程：
  Pawn → GetVelocity()     → 写入 float Speed
  Pawn → bIsCrouching      → 写入 bool bIsCrouching
  Controller → GetPitch()  → 写入 float AimPitch
  
错误的用法（常见陷阱）：
  ❌ 在 Event Graph 中直接修改 Actor 状态
  ❌ 在 Event Graph 中做复杂的游戏逻辑
  ❌ 在 Event Graph 中做大量计算（影响 Game Thread）
```

---

## 二、关键事件节点

### Blueprint Update Animation

```
这是最常用的事件，等价于 C++ 的 NativeUpdateAnimation：

事件：Blueprint Update Animation
  │
  ├── Try Get Pawn Owner → Cast To AMyCharacter
  │     └── 缓存 Character 引用（BeginPlay 时做一次即可）
  │
  ├── 速度计算
  │     GetVelocity → VectorLength → Speed（float）
  │     GetVelocity → VectorLength2D → GroundSpeed（float）
  │
  ├── 方向计算
  │     CalculateDirection(Velocity, ActorRotation) → Direction（float）
  │
  ├── 状态更新
  │     IsInAir → bIsInAir（bool）
  │     IsCrouching → bIsCrouching（bool）
  │
  └── 瞄准方向
        GetControlRotation - GetActorRotation → AimOffset（Rotator）
```

### Blueprint Begin Play

```
BeginPlay 事件：
  用途：初始化，缓存角色引用
  
  建议做：
    Try Get Pawn Owner → Cast To AMyCharacter → 存到变量 CachedCharacter
    Get Movement Component → 存到变量 CachedMovement
    
  注意：Cast 操作有性能开销，绝对不要在 Update 里每帧 Cast！
```

---

## 三、C++ 替代 Event Graph（推荐做法）

```cpp
// MyAnimInstance.h

UCLASS()
class UMyAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    // ── AnimGraph 读取的变量（BlueprintReadOnly，供蓝图 AnimGraph 访问）──

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    float Direction = 0.0f;     // -180~180，用于 Blend Space

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float AimPitch = 0.0f;      // -90~90，用于 Aim Offset

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float AimYaw = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float MovementInputAmount = 0.0f;   // 输入强度 0~1

protected:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

private:
    // 缓存引用（避免每帧 Cast）
    UPROPERTY()
    TObjectPtr<ACharacter> CachedCharacter;
    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> CachedMovement;
};

// MyAnimInstance.cpp

void UMyAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    // 初始化时缓存引用（只做一次）
    CachedCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (CachedCharacter)
        CachedMovement = CachedCharacter->GetCharacterMovement();
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if (!CachedCharacter || !CachedMovement) return;

    // 速度 & 方向
    FVector Velocity = CachedCharacter->GetVelocity();
    GroundSpeed = Velocity.Size2D();
    Direction   = CalculateDirection(Velocity, CachedCharacter->GetActorRotation());

    // 状态标志
    bIsInAir   = CachedMovement->IsFalling();
    bIsCrouching = CachedMovement->IsCrouching();
    bIsAccelerating = CachedMovement->GetCurrentAcceleration().SizeSquared() > 0.0f;

    // 输入量（0 = 无输入，1 = 满输入）
    float MaxSpeed = CachedMovement->MaxWalkSpeed;
    MovementInputAmount = FMath::Clamp(GroundSpeed / MaxSpeed, 0.0f, 1.0f);
}

void UMyAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
    // 此函数在 Worker Thread 执行
    // 只能访问上面已经写好的成员变量（已在 NativeUpdateAnimation 中赋值）
    // 不能访问 CachedCharacter（非线程安全）

    // 可以做一些衍生计算（纯数学，不访问 UObject）：
    // 例如：从 AimPitch 计算混合权重
}
```

---

## 四、瞄准方向计算

```cpp
// 计算 Aim Offset（相对于角色朝向的控制器偏移）

void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    // ... 其他变量 ...

    if (AController* Controller = CachedCharacter->GetController())
    {
        FRotator ControlRot = Controller->GetControlRotation();
        FRotator ActorRot   = CachedCharacter->GetActorRotation();

        // 计算相对旋转
        FRotator DeltaRot = ControlRot - ActorRot;
        DeltaRot.Normalize();   // 规范化到 -180~180

        // Pitch：俯仰（-90 低头，+90 仰头）
        AimPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);
        // Yaw：水平偏转（-90 左，+90 右）
        AimYaw   = FMath::ClampAngle(DeltaRot.Yaw,   -90.0f, 90.0f);
    }
}
```

---

## 五、延伸阅读

- 📄 [3.3 AnimGraph 节点系统](./03-anim-graph.md)
- 📄 [3.7 C++ AnimInstance 完整实现](./07-anim-instance-cpp.md)
- 🔗 [AnimInstance 官方 API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Animation/UAnimInstance)
