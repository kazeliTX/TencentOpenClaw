// =============================================================================
// 01_anim_instance.cpp
// 完整 C++ AnimInstance 实现（生产级模板）
// 对应文档：chapter-03-anim-blueprint/07-anim-instance-cpp.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FullAnimInstance.generated.h"

/**
 * 生产级 AnimInstance 模板
 * 特性：
 *   - 完整的 Game Thread / Worker Thread 分离
 *   - 引用缓存（无每帧 Cast）
 *   - 衍生量在 ThreadSafe 中计算
 *   - 蓝图 ReadOnly 暴露（AnimGraph 可直接访问）
 */
UCLASS(Transient, Blueprintable)
class UFullAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

    // ─── 运动变量 ───────────────────────────────
public:
    UPROPERTY(BlueprintReadOnly, Category="Anim|Move")
    float GroundSpeed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Anim|Move")
    float Direction = 0.f;          // -180~180

    UPROPERTY(BlueprintReadOnly, Category="Anim|Move")
    float MovementInputAmount = 0.f;// 0~1

    UPROPERTY(BlueprintReadOnly, Category="Anim|Move")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category="Anim|Move")
    float SpeedBlendAlpha = 0.f;    // Walk→Run 混合（ThreadSafe 计算）

    // ─── 状态变量 ───────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category="Anim|State")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category="Anim|State")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category="Anim|State")
    bool bIsDead = false;

    UPROPERTY(BlueprintReadOnly, Category="Anim|State")
    bool bIsJumping = false;

    // ─── 瞄准变量 ───────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category="Anim|Aim")
    float AimPitch = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Anim|Aim")
    float AimYaw = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Anim|Aim")
    bool bIsAiming = false;

    UPROPERTY(BlueprintReadOnly, Category="Anim|Aim")
    float SmoothedAimPitch = 0.f;   // 平滑后的 Pitch（ThreadSafe 计算）

    // ─── 次级运动 ────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category="Anim|Secondary")
    float LeanAmount = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Anim|Secondary")
    FRotator SpineTiltRot = FRotator::ZeroRotator;

protected:
    virtual void NativeInitializeAnimation() override
    {
        Super::NativeInitializeAnimation();
        CachedChar     = Cast<ACharacter>(TryGetPawnOwner());
        CachedMovement = CachedChar ? CachedChar->GetCharacterMovement() : nullptr;
    }

    // ── Game Thread ────────────────────────────────────────────────────────
    virtual void NativeUpdateAnimation(float Dt) override
    {
        Super::NativeUpdateAnimation(Dt);
        if (!CachedChar || !CachedMovement) return;

        // 运动
        const FVector Vel    = CachedChar->GetVelocity();
        GroundSpeed          = Vel.Size2D();
        Direction            = CalculateDirection(Vel, CachedChar->GetActorRotation());
        bIsAccelerating      = CachedMovement->GetCurrentAcceleration().SizeSquared() > 0.f;
        MovementInputAmount  = FMath::Clamp(
            GroundSpeed / FMath::Max(CachedMovement->MaxWalkSpeed, 1.f), 0.f, 1.f);

        // 状态
        bIsInAir    = CachedMovement->IsFalling();
        bIsCrouching = CachedMovement->IsCrouching();
        bIsJumping  = CachedChar->bPressedJump;

        // 瞄准
        if (AController* C = CachedChar->GetController())
        {
            FRotator Delta = (C->GetControlRotation() - CachedChar->GetActorRotation()).GetNormalized();
            AimPitch = FMath::ClampAngle(Delta.Pitch, -90.f, 90.f);
            AimYaw   = FMath::ClampAngle(Delta.Yaw,   -90.f, 90.f);
        }
    }

    // ── Worker Thread（线程安全，纯数学）──────────────────────────────────
    virtual void NativeThreadSafeUpdateAnimation(float Dt) override
    {
        Super::NativeThreadSafeUpdateAnimation(Dt);

        // Walk / Run 混合权重
        SpeedBlendAlpha = FMath::Clamp((GroundSpeed - 250.f) / 350.f, 0.f, 1.f);

        // 瞄准平滑（避免 Aim Offset 抖动）
        SmoothedAimPitch = FMath::FInterpTo(SmoothedAimPitch, AimPitch, Dt, 12.f);

        // 倾斜量（加速时身体前倾）
        const float TargetLean = bIsAccelerating
            ? FMath::Clamp(GroundSpeed / 600.f * 4.f, 0.f, 4.f)
            : 0.f;
        LeanAmount = FMath::FInterpTo(LeanAmount, TargetLean, Dt, 6.f);

        // 脊椎倾斜
        SpineTiltRot = FRotator(AimPitch * 0.3f, 0.f, 0.f);
    }

private:
    UPROPERTY() TObjectPtr<ACharacter>                   CachedChar;
    UPROPERTY() TObjectPtr<UCharacterMovementComponent>  CachedMovement;
};

// =============================================================================
// 使用说明：
//   1. 将此 AnimInstance 子类设为你的 AnimBP 的 Parent Class
//   2. AnimBP 中直接使用 Get GroundSpeed / Get Direction 等节点
//   3. AnimBP 的 Event Graph 可以基本清空（C++ 已处理所有变量）
// =============================================================================
