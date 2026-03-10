// =============================================================================
// 01_replicated_anim_instance.cpp
// 完整的网络同步 AnimInstance 示例
// 对应文档：chapter-09-network/03-anim-variable-replication.md
// =============================================================================
#pragma once
#include "Animation/AnimInstance.h"
#include "ReplicatedAnimInstance.generated.h"

UCLASS()
class MYPROJECT_API UReplicatedAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    // AnimGraph 直接读取的变量（Fast Path 兼容）
    UPROPERTY(BlueprintReadOnly) float   Speed         = 0.f;
    UPROPERTY(BlueprintReadOnly) float   Direction     = 0.f;
    UPROPERTY(BlueprintReadOnly) float   AimPitch      = 0.f;
    UPROPERTY(BlueprintReadOnly) float   AimYaw        = 0.f;
    UPROPERTY(BlueprintReadOnly) bool    bIsInAir      = false;
    UPROPERTY(BlueprintReadOnly) bool    bIsInCombat   = false;
    UPROPERTY(BlueprintReadOnly) bool    bIsSimProxy   = false;
    UPROPERTY(BlueprintReadOnly) uint8   WeaponType    = 0;

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override
    {
        ACharacter* C = Cast<ACharacter>(TryGetPawnOwner());
        if (!C) return;

        bIsSimProxy = (C->GetLocalRole() == ROLE_SimulatedProxy);
        bIsInAir    = C->GetCharacterMovement()->IsFalling();
        WeaponType  = Cast<AMyCharacter>(C) ? Cast<AMyCharacter>(C)->WeaponType : 0;
        bIsInCombat = Cast<AMyCharacter>(C) ? Cast<AMyCharacter>(C)->bIsInCombat : false;

        FVector Velocity;
        FRotator ControlRot;

        if (C->IsLocallyControlled())
        {
            // AutProxy/Server：直接读本地数据
            Velocity   = C->GetVelocity();
            AController* PC = C->GetController();
            ControlRot = PC ? PC->GetControlRotation() : C->GetActorRotation();
        }
        else
        {
            // SimProxy：读网络同步数据
            Velocity = C->GetVelocity(); // CMC 自动同步
            float RemotePitch = (float)C->RemoteViewPitch / 255.f * 360.f;
            if (RemotePitch > 180.f) RemotePitch -= 360.f;
            ControlRot = FRotator(RemotePitch, C->GetActorRotation().Yaw, 0.f);
        }

        Speed     = Velocity.Size2D();
        AimPitch  = FMath::Clamp(ControlRot.Pitch, -90.f, 90.f);

        // 计算相对方向（用于 Strafe 动画）
        FVector NormVel = Velocity.GetSafeNormal2D();
        FVector Forward = C->GetActorForwardVector();
        Direction = FMath::RadiansToDegrees(FMath::Atan2(
            FVector::DotProduct(NormVel, C->GetActorRightVector()),
            FVector::DotProduct(NormVel, Forward)));
    }
};
