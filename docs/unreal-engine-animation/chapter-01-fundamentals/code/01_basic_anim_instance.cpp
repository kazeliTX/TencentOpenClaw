// =============================================================================
// 01_basic_anim_instance.cpp
// 基础 AnimInstance 示例
// 演示：AnimInstance 生命周期、变量更新、线程安全最佳实践
// 对应文档：chapter-01-fundamentals/01-animation-system-overview.md
// =============================================================================

#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BasicAnimInstance.generated.h"

// ─────────────────────────────────────────────────────────────
// AnimInstance 类定义
// ─────────────────────────────────────────────────────────────

UCLASS()
class MYPROJECT_API UBasicAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    // ── 运动相关变量（暴露给 AnimGraph 蓝图）──

    /** 角色水平速度（cm/s），用于 Blend Space Y 轴 */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion",
              meta = (DisplayName = "Speed"))
    float Speed = 0.0f;

    /** 移动方向（相对角色朝向，-180~180 度），用于 Blend Space X 轴 */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion",
              meta = (DisplayName = "Direction"))
    float Direction = 0.0f;

    /** 是否在空中（跳跃/下落） */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion",
              meta = (DisplayName = "Is In Air"))
    bool bIsInAir = false;

    /** 是否在蹲伏 */
    UPROPERTY(BlueprintReadOnly, Category = "Locomotion",
              meta = (DisplayName = "Is Crouching"))
    bool bIsCrouching = false;

    // ── 瞄准相关变量 ──

    /** 瞄准水平角（-90~90），用于 Aim Offset X 轴 */
    UPROPERTY(BlueprintReadOnly, Category = "Aiming",
              meta = (DisplayName = "Aim Yaw"))
    float AimYaw = 0.0f;

    /** 瞄准俯仰角（-90~90），用于 Aim Offset Y 轴 */
    UPROPERTY(BlueprintReadOnly, Category = "Aiming",
              meta = (DisplayName = "Aim Pitch"))
    float AimPitch = 0.0f;

protected:
    // ─────────────────────────────────────────────────────────
    // 生命周期函数
    // ─────────────────────────────────────────────────────────

    /**
     * 初始化 —— 仅调用一次（对应蓝图的 Blueprint Initialize Animation）
     * 在这里缓存对 Owner 的引用，避免每帧 Cast
     */
    virtual void NativeInitializeAnimation() override
    {
        Super::NativeInitializeAnimation();

        // 缓存 Owner（一次 Cast，之后直接用）
        OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
        
        if (OwnerCharacter)
        {
            // 缓存 MovementComponent（高频访问，值得缓存）
            MovementComp = OwnerCharacter->GetCharacterMovement();
        }
    }

    /**
     * 游戏线程更新 —— 每帧调用（对应蓝图的 Blueprint Update Animation）
     * 适合做：访问 Actor/Component、调用 Gameplay 函数
     * 不适合：大量计算（移到 NativeThreadSafeUpdateAnimation）
     */
    virtual void NativeUpdateAnimation(float DeltaSeconds) override
    {
        Super::NativeUpdateAnimation(DeltaSeconds);

        if (!OwnerCharacter || !MovementComp) return;

        // 从游戏线程读取数据，存入缓存变量
        // （这些缓存变量将在 Worker Thread 中被读取）
        CachedVelocity       = OwnerCharacter->GetVelocity();
        CachedActorRotation  = OwnerCharacter->GetActorRotation();
        CachedAimRotation    = OwnerCharacter->GetBaseAimRotation();
        CachedIsInAir        = MovementComp->IsFalling();
        CachedIsCrouching    = MovementComp->IsCrouching();
    }

    /**
     * 工作线程安全更新 —— UE5 新增，在 Worker Thread 执行（性能更好）
     * 适合做：基于缓存数据的纯计算
     * 禁止：访问 UObject、调用任何非线程安全函数
     */
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override
    {
        Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

        // ── 更新速度 ──
        Speed = CachedVelocity.Size2D();  // 只取水平速度，忽略 Z 轴

        // ── 更新方向 ──
        if (Speed > 1.0f)
        {
            // 计算速度方向相对于角色朝向的角度
            FVector NormalVelocity = CachedVelocity.GetSafeNormal2D();
            FVector Forward = CachedActorRotation.Vector();
            FVector Right   = FRotationMatrix(CachedActorRotation).GetScaledAxis(EAxis::Y);

            float ForwardDot = FVector::DotProduct(NormalVelocity, Forward);
            float RightDot   = FVector::DotProduct(NormalVelocity, Right);
            Direction = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
        }
        else
        {
            // 静止时方向归零（可选：平滑归零）
            Direction = FMath::FInterpTo(Direction, 0.0f, DeltaSeconds, 5.0f);
        }

        // ── 更新跳跃/蹲伏状态 ──
        bIsInAir    = CachedIsInAir;
        bIsCrouching = CachedIsCrouching;

        // ── 更新瞄准角度 ──
        FRotator DeltaRot = (CachedAimRotation - CachedActorRotation).GetNormalized();
        float TargetYaw   = FMath::ClampAngle(DeltaRot.Yaw,   -90.0f, 90.0f);
        float TargetPitch = FMath::ClampAngle(DeltaRot.Pitch, -90.0f, 90.0f);

        // 平滑插值，避免突变
        AimYaw   = FMath::FInterpTo(AimYaw,   TargetYaw,   DeltaSeconds, 15.0f);
        AimPitch = FMath::FInterpTo(AimPitch, TargetPitch, DeltaSeconds, 15.0f);
    }

private:
    // ─────────────────────────────────────────────────────────
    // 缓存的 Owner 引用（避免每帧 Cast）
    // ─────────────────────────────────────────────────────────

    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter = nullptr;

    UPROPERTY()
    TObjectPtr<UCharacterMovementComponent> MovementComp = nullptr;

    // ─────────────────────────────────────────────────────────
    // 游戏线程 → 工作线程 的数据桥梁
    // 在 NativeUpdateAnimation（游戏线程）写入
    // 在 NativeThreadSafeUpdateAnimation（工作线程）读取
    // ─────────────────────────────────────────────────────────

    FVector  CachedVelocity      = FVector::ZeroVector;
    FRotator CachedActorRotation = FRotator::ZeroRotator;
    FRotator CachedAimRotation   = FRotator::ZeroRotator;
    bool     CachedIsInAir       = false;
    bool     CachedIsCrouching   = false;
};

// =============================================================================
// 使用说明：
//
// 1. 在你的 AnimBP 中，设置 Parent Class 为 UBasicAnimInstance
//
// 2. AnimGraph 中可以直接使用：
//    • Speed     → Blend Space Y 轴（0~600）
//    • Direction → Blend Space X 轴（-180~180）
//    • bIsInAir  → 状态机转换条件
//    • AimYaw/AimPitch → Aim Offset 输入
//
// 3. 状态机配置示例：
//    Idle → Moving:   Speed > 10.0
//    Moving → Idle:   Speed < 5.0
//    * → InAir:       bIsInAir == true
//    InAir → Ground:  bIsInAir == false
//
// 4. 编译后在 AnimBP 的 Class Settings 里将 Parent Class 改为此类
// =============================================================================
