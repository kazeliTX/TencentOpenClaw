// =============================================================================
// 03_ragdoll_controller.cpp
// 布娃娃启停控制（完整实现）
// 对应文档：chapter-02-skeleton-system/07-ragdoll-system.md
// =============================================================================

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "RagdollControllerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRagdollDisabled);

UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API URagdollControllerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URagdollControllerComponent()
    {
        PrimaryComponentTick.bCanEverTick = true;
        PrimaryComponentTick.bStartWithTickEnabled = false;
    }

    // ── 事件 ──
    UPROPERTY(BlueprintAssignable) FOnRagdollEnabled OnRagdollEnabled;
    UPROPERTY(BlueprintAssignable) FOnRagdollDisabled OnRagdollDisabled;

    /** 启用布娃娃（带初始冲量）*/
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnableRagdoll(FVector InitialImpulse = FVector::ZeroVector)
    {
        if (bRagdollActive || !CachedMesh) return;

        bRagdollActive = true;

        // 停止角色移动
        if (CachedMovement)
        {
            CachedMovement->DisableMovement();
            CachedMovement->StopMovementImmediately();
        }

        // 禁用胶囊体碰撞（避免阻挡布娃娃）
        if (CachedCapsule)
        {
            CachedCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // 保存当前 Mesh 相对变换（恢复时用）
        SavedMeshRelTransform = CachedMesh->GetRelativeTransform();

        // Detach 让 Mesh 自由飞
        CachedMesh->DetachFromComponent(
            FDetachmentTransformRules::KeepWorldTransform);

        // 开启全身物理
        CachedMesh->SetAllBodiesSimulatePhysics(true);
        CachedMesh->SetSimulatePhysics(true);
        CachedMesh->WakeAllRigidBodies();
        CachedMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CachedMesh->SetCollisionProfileName(FName("Ragdoll"));

        // 施加初始冲量
        if (!InitialImpulse.IsNearlyZero())
        {
            CachedMesh->AddImpulse(InitialImpulse, NAME_None, true);
        }

        // 开启 Tick（用于跟踪骨盆位置，让胶囊体跟随）
        SetComponentTickEnabled(true);

        OnRagdollEnabled.Broadcast();
    }

    /** 停用布娃娃，恢复动画 */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DisableRagdoll()
    {
        if (!bRagdollActive || !CachedMesh) return;

        bRagdollActive = false;
        SetComponentTickEnabled(false);

        // 1. 将 Actor 移动到骨盆当前位置
        ACharacter* Owner = Cast<ACharacter>(GetOwner());
        if (Owner)
        {
            FVector PelvisWorld = CachedMesh->GetBoneLocation(FName("pelvis"));
            float HalfHeight = CachedCapsule
                ? CachedCapsule->GetScaledCapsuleHalfHeight()
                : 90.0f;
            FVector NewActorLoc = PelvisWorld;
            NewActorLoc.Z -= HalfHeight;

            // 地面检测：确保不是悬空
            FHitResult GroundHit;
            FVector TraceStart = PelvisWorld;
            FVector TraceEnd   = PelvisWorld - FVector(0, 0, 200.0f);
            if (GetWorld()->LineTraceSingleByChannel(
                GroundHit, TraceStart, TraceEnd,
                ECC_WorldStatic))
            {
                NewActorLoc.Z = GroundHit.ImpactPoint.Z + HalfHeight;
            }

            Owner->SetActorLocation(NewActorLoc, false, nullptr,
                ETeleportType::TeleportPhysics);
        }

        // 2. 关闭物理
        CachedMesh->SetSimulatePhysics(false);
        CachedMesh->SetAllBodiesSimulatePhysics(false);
        CachedMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CachedMesh->SetCollisionProfileName(FName("CharacterMesh"));

        // 3. 重新 Attach 到根组件
        if (Owner)
        {
            CachedMesh->AttachToComponent(
                Owner->GetRootComponent(),
                FAttachmentTransformRules::SnapToTargetNotIncludingScale
            );
            CachedMesh->SetRelativeTransform(SavedMeshRelTransform);
        }

        // 4. 恢复移动
        if (CachedMovement)
        {
            CachedMovement->SetMovementMode(MOVE_Walking);
        }

        // 5. 恢复胶囊体
        if (CachedCapsule)
        {
            CachedCapsule->SetCollisionEnabled(
                ECollisionEnabled::QueryAndPhysics);
        }

        OnRagdollDisabled.Broadcast();
    }

    /** 当前是否处于布娃娃状态 */
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return bRagdollActive; }

    /** 对特定骨骼施加冲量（无需启用完整布娃娃）*/
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void ApplyImpulseToBone(FName BoneName, FVector Impulse, bool bVelocityChange = true)
    {
        if (CachedMesh)
        {
            CachedMesh->AddImpulse(Impulse, BoneName, bVelocityChange);
        }
    }

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        ACharacter* Owner = Cast<ACharacter>(GetOwner());
        if (!Owner) return;

        CachedMesh      = Owner->GetMesh();
        CachedCapsule   = Owner->GetCapsuleComponent();
        CachedMovement  = Owner->GetCharacterMovement();
    }

    // Tick：在布娃娃激活时，让胶囊体跟随骨盆（保持 Actor 位置同步）
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override
    {
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
        if (!bRagdollActive || !CachedMesh || !GetOwner()) return;

        FVector PelvisLoc = CachedMesh->GetBoneLocation(FName("pelvis"));
        FVector CurrentLoc = GetOwner()->GetActorLocation();

        // 平滑跟随骨盆（水平方向）
        FVector TargetLoc = FMath::VInterpTo(CurrentLoc, PelvisLoc, DeltaTime, 10.0f);
        TargetLoc.Z = CurrentLoc.Z;  // 保持 Z 位置（由胶囊体负责）
        GetOwner()->SetActorLocation(TargetLoc, false, nullptr,
            ETeleportType::TeleportPhysics);
    }

private:
    bool bRagdollActive = false;
    FTransform SavedMeshRelTransform;

    UPROPERTY() TObjectPtr<USkeletalMeshComponent>        CachedMesh;
    UPROPERTY() TObjectPtr<UCapsuleComponent>              CachedCapsule;
    UPROPERTY() TObjectPtr<UCharacterMovementComponent>    CachedMovement;
};
