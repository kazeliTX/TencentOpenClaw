// =============================================================================
// 01_ragdoll_system.cpp
// 完整 Ragdoll 系统：激活/死亡过渡/起身/受击/武器脱手
// 对应文档：chapter-05/04~06
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/PoseSnapshot.h"
#include "RagdollCharacter.generated.h"

UENUM(BlueprintType)
enum class ERagdollState : uint8
{
    Active,        // 正常活跃
    Dying,         // 死亡动画播放中
    Ragdoll,       // 完整布娃娃
    GettingUp,     // 起身中
};

UCLASS()
class ARagdollCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Ragdoll") UAnimMontage* DeathMontage;
    UPROPERTY(EditAnywhere, Category="Ragdoll") UAnimMontage* GetUpFrontMontage;
    UPROPERTY(EditAnywhere, Category="Ragdoll") UAnimMontage* GetUpBackMontage;
    UPROPERTY(EditAnywhere, Category="Ragdoll") AActor*       EquippedWeapon;

    ERagdollState RagdollState = ERagdollState::Active;
    FPoseSnapshot SavedRagdollPose;
    FTimerHandle  BlendTimer;
    float         PhysicsBlendTarget = 0.f;

    // ─────────────────────────────────────────────
    // 角色死亡入口
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Ragdoll")
    void Die()
    {
        if (RagdollState != ERagdollState::Active) return;
        RagdollState = ERagdollState::Dying;

        GetCharacterMovement()->DisableMovement();
        GetCharacterMovement()->StopMovementImmediately();

        if (DeathMontage)
            PlayAnimMontage(DeathMontage);

        // 先启动物理但权重=0（预热）
        GetMesh()->SetAllBodiesSimulatePhysics(true);
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->PhysicsBlendWeight = 0.f;

        // 0.25s 后开始混入布娃娃
        GetWorld()->GetTimerManager().SetTimer(BlendTimer,
            FTimerDelegate::CreateLambda([this]()
            {
                StartBlendToRagdoll(0.4f);
                // 关闭胶囊碰撞
                GetCapsuleComponent()->SetCollisionEnabled(
                    ECollisionEnabled::NoCollision);
                // 武器脱手
                DropWeapon();
            }), 0.25f, false);
    }

    // ─────────────────────────────────────────────
    // 渐变到布娃娃
    // ─────────────────────────────────────────────
    void StartBlendToRagdoll(float Duration)
    {
        RagdollState = ERagdollState::Ragdoll;
        GetMesh()->SetCollisionProfileName(FName("Ragdoll"));

        float StepDt = 0.033f;
        int32 Steps  = FMath::CeilToInt(Duration / StepDt);
        float StepA  = 1.f / (float)Steps;
        int32* Cnt   = new int32(0);

        GetWorld()->GetTimerManager().SetTimer(BlendTimer,
            FTimerDelegate::CreateLambda([this, Steps, StepA, Cnt]()
            {
                ++(*Cnt);
                GetMesh()->PhysicsBlendWeight =
                    FMath::Min((*Cnt) * StepA, 1.f);
                if (*Cnt >= Steps)
                {
                    delete Cnt;
                    GetWorld()->GetTimerManager().ClearTimer(BlendTimer);
                }
            }), StepDt, true);

        // 继承角色速度
        FVector Vel = GetVelocity();
        GetMesh()->SetPhysicsLinearVelocity(Vel, false, NAME_None);
    }

    // ─────────────────────────────────────────────
    // 起身
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Ragdoll")
    void GetUp()
    {
        if (RagdollState != ERagdollState::Ragdoll) return;
        RagdollState = ERagdollState::GettingUp;

        // 判断朝向（面朝上/朝下）
        FVector PelvisUp = GetMesh()->GetBoneAxis("pelvis", EBoneAxis::BA_Z);
        bool bFaceUp = FVector::DotProduct(PelvisUp, FVector::UpVector) > 0.f;
        UAnimMontage* Montage = bFaceUp ? GetUpFrontMontage : GetUpBackMontage;

        // 保存布娃娃姿势
        GetMesh()->SnapshotPose(SavedRagdollPose);

        // 关闭物理
        GetMesh()->SetSimulatePhysics(false);
        GetMesh()->SetAllBodiesSimulatePhysics(false);
        GetMesh()->PhysicsBlendWeight = 1.f;

        // 把 Actor 位置移到骨盆正上方
        FVector PelvisLoc = GetMesh()->GetBoneLocation(FName("pelvis"));
        FVector NewActorLoc = PelvisLoc;
        NewActorLoc.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        SetActorLocation(NewActorLoc, false, nullptr,
            ETeleportType::TeleportPhysics);

        // 恢复胶囊
        GetCapsuleComponent()->SetCollisionEnabled(
            ECollisionEnabled::QueryAndPhysics);
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);

        // 播放起身动画
        if (Montage) PlayAnimMontage(Montage);

        // 1.5s 后完全恢复
        GetWorld()->GetTimerManager().SetTimer(BlendTimer,
            FTimerDelegate::CreateLambda([this]()
            {
                GetMesh()->PhysicsBlendWeight = 0.f;
                GetMesh()->SetCollisionProfileName(FName("CharacterMesh"));
                RagdollState = ERagdollState::Active;
            }), 1.5f, false);
    }

    // ─────────────────────────────────────────────
    // 武器脱手飞出
    // ─────────────────────────────────────────────
    void DropWeapon()
    {
        if (!EquippedWeapon) return;
        UPrimitiveComponent* WC =
            EquippedWeapon->FindComponentByClass<UPrimitiveComponent>();
        if (!WC) return;

        EquippedWeapon->DetachFromActor(
            FDetachmentTransformRules::KeepWorldTransform);
        WC->SetSimulatePhysics(true);
        WC->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        FVector HandVel = GetMesh()->GetPhysicsLinearVelocityAtPoint(
            GetMesh()->GetBoneLocation(FName("hand_r")));
        WC->SetPhysicsLinearVelocity(HandVel);
        EquippedWeapon->SetLifeSpan(10.f);
        EquippedWeapon = nullptr;
    }

    // ─────────────────────────────────────────────
    // 每帧追踪布娃娃根骨骼位置（保持 Actor 在尸体上方）
    // ─────────────────────────────────────────────
    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);
        if (RagdollState == ERagdollState::Ragdoll)
        {
            FVector PelvisLoc =
                GetMesh()->GetBoneLocation(FName("pelvis"));
            float HalfH =
                GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
            FVector TargetLoc = PelvisLoc + FVector(0, 0, HalfH);
            SetActorLocation(
                FMath::VInterpTo(GetActorLocation(), TargetLoc, Dt, 20.f),
                false, nullptr, ETeleportType::TeleportPhysics);
        }
    }
};
