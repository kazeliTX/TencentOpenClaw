// =============================================================================
// 01_ragdoll_controller.cpp
// 完整布娃娃控制器（触发 + 静止检测 + Pose Snapshot 恢复）
// 对应文档：chapter-07-physics-animation/02-ragdoll-system.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/PoseSnapshot.h"
#include "RagdollController.generated.h"

UENUM(BlueprintType)
enum class ERagdollState : uint8 { Inactive, Active, Recovering };

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API URagdollController : public UActorComponent
{
    GENERATED_BODY()
public:
    URagdollController() { PrimaryComponentTick.bCanEverTick = true; }

    UPROPERTY(EditAnywhere, Category="Ragdoll") TObjectPtr<UAnimMontage> GetUpFront;
    UPROPERTY(EditAnywhere, Category="Ragdoll") TObjectPtr<UAnimMontage> GetUpBack;
    UPROPERTY(EditAnywhere, Category="Ragdoll") float StillSpeedThreshold = 10.f;
    UPROPERTY(EditAnywhere, Category="Ragdoll") float StillRequiredTime    = 1.0f;
    UPROPERTY(EditAnywhere, Category="Ragdoll") float MaxRagdollTime       = 8.0f;

    // AnimInstance 读取此快照
    UPROPERTY(BlueprintReadOnly, Category="Ragdoll") FPoseSnapshot RagdollSnapshot;
    UPROPERTY(BlueprintReadOnly, Category="Ragdoll") ERagdollState State = ERagdollState::Inactive;

    UFUNCTION(BlueprintCallable, Category="Ragdoll")
    void Activate(FVector Impulse = FVector::ZeroVector)
    {
        ACharacter* C = GetCharacter();
        if (!C || State != ERagdollState::Inactive) return;

        // 停止动画
        if (auto* Anim = C->GetMesh()->GetAnimInstance())
            Anim->Montage_StopAll(0.1f);

        // 禁用胶囊 + 移动
        C->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        C->GetCharacterMovement()->DisableMovement();

        // 开物理
        C->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
        C->GetMesh()->SetSimulatePhysics(true);
        C->GetMesh()->SetAllBodiesBelowSimulatePhysics(NAME_None, true, true);

        if (!Impulse.IsNearlyZero())
            C->GetMesh()->AddImpulse(Impulse, NAME_None, true);

        State       = ERagdollState::Active;
        StillTime   = 0.f;
        ActiveTime  = 0.f;
    }

    UFUNCTION(BlueprintCallable, Category="Ragdoll")
    void ForceRecover() { TryRecover(); }

protected:
    virtual void TickComponent(float Dt, ELevelTick, FActorComponentTickFunction*) override
    {
        if (State != ERagdollState::Active) return;

        ActiveTime += Dt;

        // 超时强制恢复
        if (ActiveTime >= MaxRagdollTime) { TryRecover(); return; }

        // 静止检测
        ACharacter* C = GetCharacter();
        if (!C) return;
        float Speed = C->GetMesh()->GetBoneLinearVelocity(FName("pelvis")).Size();
        StillTime   = (Speed < StillSpeedThreshold) ? StillTime + Dt : 0.f;
        if (StillTime >= StillRequiredTime) TryRecover();
    }

private:
    float StillTime  = 0.f;
    float ActiveTime = 0.f;

    void TryRecover()
    {
        ACharacter* C = GetCharacter();
        if (!C || State != ERagdollState::Active) return;

        // 捕获快照（停物理前！）
        if (auto* Anim = C->GetMesh()->GetAnimInstance())
            Anim->SnapshotPose(RagdollSnapshot);

        // 判断朝向
        bool bFaceDown = FVector::DotProduct(
            C->GetMesh()->GetBoneAxis(FName("pelvis"), EBoneAxis::BA_Z),
            FVector::UpVector) < 0.f;

        // 关物理
        C->GetMesh()->SetSimulatePhysics(false);
        C->GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));

        // 恢复移动
        C->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
        C->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // 对齐胶囊到骨盆
        FVector PelvisLoc = C->GetMesh()->GetBoneLocation(FName("pelvis"));
        C->SetActorLocation(PelvisLoc + FVector(0,0,C->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

        // 播放起立 Montage
        UAnimMontage* Montage = bFaceDown ? GetUpFront : GetUpBack;
        if (Montage) C->PlayAnimMontage(Montage);

        State = ERagdollState::Recovering;
    }

    ACharacter* GetCharacter() const { return Cast<ACharacter>(GetOwner()); }
};
