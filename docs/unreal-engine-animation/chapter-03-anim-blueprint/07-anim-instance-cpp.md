# 3.7 C++ AnimInstance 完整实现

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 40 分钟
> 这是中大型项目必备的最佳实践

---

## 一、为什么要用 C++ AnimInstance

```
纯蓝图 AnimBP 的问题：
  1. 蓝图 VM 开销：每个节点执行有虚拟机解释开销
  2. Cast 陷阱：开发者常忘记缓存引用，每帧 Cast
  3. 线程安全难保证：蓝图难以正确区分 Game Thread / Worker Thread
  4. 维护困难：大量蓝图节点连线，难以 Code Review

C++ AnimInstance 的优势：
  1. 零虚拟机开销（直接 CPU 执行）
  2. 强制线程安全分离（编译器报错）
  3. 易于 Code Review 和版本控制
  4. AnimGraph 仍可视化（C++ 提供数据，蓝图负责节点逻辑）
```

---

## 二、完整 C++ AnimInstance 模板

```cpp
// ─────────────────────────────────────────────────
// MyAnimInstance.h
// ─────────────────────────────────────────────────
#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyAnimInstance.generated.h"

class AMyCharacter;
class UCharacterMovementComponent;

/**
 * 角色动画实例
 * Game Thread 逻辑（NativeUpdateAnimation）分离于
 * Worker Thread 骨骼计算（AnimGraph + NativeThreadSafeUpdateAnimation）
 */
UCLASS(Transient, Blueprintable)
class MYPROJECT_API UMyAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

    // ═══════════════════════════════════════════════
    // AnimGraph 可读变量（对蓝图只读，线程安全）
    // ═══════════════════════════════════════════════
public:
    // ── 移动 ──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Movement", meta=(AllowPrivateAccess))
    float GroundSpeed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Movement", meta=(AllowPrivateAccess))
    float Direction = 0.0f;         // -180 ~ 180，用于 Blend Space

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Movement", meta=(AllowPrivateAccess))
    float MovementInputAmount = 0.0f; // 0~1，表示输入强度

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Movement", meta=(AllowPrivateAccess))
    bool bIsAccelerating = false;

    // ── 状态 ──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|State", meta=(AllowPrivateAccess))
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State", meta=(AllowPrivateAccess))
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State", meta=(AllowPrivateAccess))
    bool bIsDead = false;

    UPROPERTY(BlueprintReadOnly, Category = "Anim|State", meta=(AllowPrivateAccess))
    bool bIsJumping = false;         // 是否处于主动跳跃（按下跳跃键时）

    // ── 瞄准 ──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aim", meta=(AllowPrivateAccess))
    float AimPitch = 0.0f;           // -90 ~ 90

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aim", meta=(AllowPrivateAccess))
    float AimYaw = 0.0f;             // -90 ~ 90

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Aim", meta=(AllowPrivateAccess))
    bool bIsAiming = false;

    // ── 移动细节（衍生量，Worker Thread 中计算）──
    UPROPERTY(BlueprintReadOnly, Category = "Anim|Derived", meta=(AllowPrivateAccess))
    float LeanAmount = 0.0f;         // 身体倾斜量（加速时前倾）

    UPROPERTY(BlueprintReadOnly, Category = "Anim|Derived", meta=(AllowPrivateAccess))
    float SpeedBlendAlpha = 0.0f;    // Walk↔Run 混合 Alpha

    // ═══════════════════════════════════════════════
    // 生命周期
    // ═══════════════════════════════════════════════
protected:
    /** 初始化：缓存引用 */
    virtual void NativeInitializeAnimation() override;

    /** Game Thread：读取 Gameplay 状态，更新变量 */
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    /** Worker Thread：纯数学计算，派生量 */
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

    /** 动画评估完成后（骨骼变换已计算）*/
    virtual void NativePostEvaluateAnimation() override;

    // ═══════════════════════════════════════════════
    // 私有：缓存引用（不暴露给蓝图）
    // ═══════════════════════════════════════════════
private:
    UPROPERTY()
    TObjectPtr<AMyCharacter> CachedCharacter;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> CachedMovement;

    // 用于计算倾斜量的历史速度
    FVector PrevVelocity = FVector::ZeroVector;

    // 辅助函数
    void UpdateLocomotion(float DeltaSeconds);
    void UpdateAiming(float DeltaSeconds);
    void UpdateState(float DeltaSeconds);
};
```

---

## 三、完整实现

```cpp
// ─────────────────────────────────────────────────
// MyAnimInstance.cpp
// ─────────────────────────────────────────────────
#include "MyAnimInstance.h"
#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMyAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // 只做一次 Cast（初始化时）
    CachedCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
    if (CachedCharacter)
    {
        CachedMovement = CachedCharacter->GetCharacterMovement();
    }
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    // Guard：引用无效时提前返回
    if (!CachedCharacter || !CachedMovement) return;

    // 分类更新
    UpdateLocomotion(DeltaSeconds);
    UpdateAiming(DeltaSeconds);
    UpdateState(DeltaSeconds);
}

void UMyAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
    FVector Velocity = CachedCharacter->GetVelocity();

    // 地面速度（忽略 Z 轴）
    GroundSpeed = Velocity.Size2D();

    // 运动方向（相对于角色朝向，-180~180）
    Direction = CalculateDirection(Velocity, CachedCharacter->GetActorRotation());

    // 输入状态
    bIsAccelerating =
        CachedMovement->GetCurrentAcceleration().SizeSquared() > SMALL_NUMBER;

    // 输入量（0~1）
    const float MaxSpeed = CachedMovement->MaxWalkSpeed;
    MovementInputAmount = (MaxSpeed > 0.0f)
        ? FMath::Clamp(GroundSpeed / MaxSpeed, 0.0f, 1.0f)
        : 0.0f;

    // 保存速度历史（供 Worker Thread 计算倾斜）
    PrevVelocity = Velocity;
}

void UMyAnimInstance::UpdateAiming(float DeltaSeconds)
{
    AController* Controller = CachedCharacter->GetController();
    if (!Controller) return;

    FRotator ControlRot = Controller->GetControlRotation();
    FRotator ActorRot   = CachedCharacter->GetActorRotation();
    FRotator Delta      = (ControlRot - ActorRot).GetNormalized();

    // Pitch: -90（低头）~ +90（仰头）
    AimPitch = FMath::ClampAngle(Delta.Pitch, -90.0f, 90.0f);
    // Yaw: -90（左转）~ +90（右转）
    AimYaw   = FMath::ClampAngle(Delta.Yaw,   -90.0f, 90.0f);

    // 瞄准状态（由角色设置）
    bIsAiming = CachedCharacter->bIsAiming;
}

void UMyAnimInstance::UpdateState(float DeltaSeconds)
{
    bIsInAir    = CachedMovement->IsFalling();
    bIsCrouching = CachedMovement->IsCrouching();
    bIsJumping  = CachedCharacter->bPressedJump;
    bIsDead     = CachedCharacter->IsDead();    // 假设有此函数
}

void UMyAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

    // ⚠️ 注意：这里不能访问 CachedCharacter 等 UObject
    // 只能使用上面已经写好的成员变量（值类型，线程安全）

    // 计算 Walk↔Run 混合 Alpha
    const float WalkSpeed = 300.0f;
    const float RunSpeed  = 600.0f;
    SpeedBlendAlpha = (RunSpeed > WalkSpeed)
        ? FMath::Clamp((GroundSpeed - WalkSpeed) / (RunSpeed - WalkSpeed), 0.0f, 1.0f)
        : 0.0f;

    // 计算身体倾斜量（加速方向上的前倾）
    // 简化实现：根据 bIsAccelerating 和 GroundSpeed 估算
    const float TargetLean = bIsAccelerating ? FMath::Clamp(GroundSpeed / 600.0f * 5.0f, 0.0f, 5.0f) : 0.0f;
    LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, DeltaSeconds, 6.0f);
}

void UMyAnimInstance::NativePostEvaluateAnimation()
{
    Super::NativePostEvaluateAnimation();
    // 骨骼变换已计算完毕，此处可读取骨骼位置（如脚部 IK 的地面检测）
}
```

---

## 四、在蓝图 AnimBP 中使用 C++ 变量

```
C++ AnimInstance 与蓝图 AnimGraph 的配合：

1. 将 AnimBP 的父类设为你的 C++ 类：
   AnimBP → Class Settings → Parent Class: UMyAnimInstance

2. 在 AnimGraph 中直接访问 C++ 变量：
   蓝图节点 → 右键 → 搜索变量名（如 "GroundSpeed"）→ Get GroundSpeed
   → 连接到 Blend Space 的 Speed 输入

3. 在 Event Graph 中：
   基本上可以清空（所有逻辑已在 C++ 中）
   或只保留少量蓝图特有的操作
```

---

## 五、延伸阅读

- 🔗 [UAnimInstance API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Animation/UAnimInstance)
- 🔗 [AnimBP 最佳实践（Epic官方）](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-best-practices-in-unreal-engine)
- 📄 [代码：完整 AnimInstance 模板](./code/01_anim_instance.cpp)
