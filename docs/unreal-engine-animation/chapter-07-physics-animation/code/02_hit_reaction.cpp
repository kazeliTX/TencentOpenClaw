// =============================================================================
// 02_hit_reaction.cpp
// 物理受击反应系统（Physical Animation + 冲量）
// 对应文档：chapter-07-physics-animation/04-hit-reaction.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "HitReactionComponent.generated.h"

UENUM(BlueprintType)
enum class EHitDirection : uint8
{ Front, Back, Left, Right, FrontLeft, FrontRight, BackLeft, BackRight };

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UHitReactionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UHitReactionComponent() { PrimaryComponentTick.bCanEverTick = true; }

    UPROPERTY(EditAnywhere, Category="HitReact") float HitPhysicsWeight  = 0.15f;
    UPROPERTY(EditAnywhere, Category="HitReact") float RecoveryDuration  = 0.35f;
    UPROPERTY(EditAnywhere, Category="HitReact") float MinImpulse        = 200.f;
    UPROPERTY(EditAnywhere, Category="HitReact") float MaxImpulse        = 2000.f;

    UFUNCTION(BlueprintCallable, Category="HitReact")
    void ReactToHit(FVector HitWorldLocation, FVector HitImpulse)
    {
        ACharacter* C = Cast<ACharacter>(GetOwner());
        if (!C) return;

        USkeletalMeshComponent* Mesh = C->GetMesh();
        float Strength = HitImpulse.Size();
        if (Strength < MinImpulse) return;

        // 找到最近骨骼
        FName HitBone = Mesh->FindClosestBone(HitWorldLocation);

        // 开启骨骼物理
        Mesh->SetAllBodiesBelowSimulatePhysics(HitBone, true, true);
        Mesh->SetAllBodiesBelowPhysicsBlendWeight(HitBone, HitPhysicsWeight, false, true);

        // 施加冲量
        float ClampedStr = FMath::Clamp(Strength, MinImpulse, MaxImpulse);
        FVector FinalImpulse = HitImpulse.GetSafeNormal() * ClampedStr;
        Mesh->AddImpulseAtLocation(FinalImpulse, HitWorldLocation);

        // 开始恢复
        RecoveryAlpha    = 0.f;
        bIsRecovering    = true;
        ActiveHitBone    = HitBone;

        float RecoveryTime = FMath::GetMappedRangeValueClamped(
            FVector2D(MinImpulse, MaxImpulse),
            FVector2D(0.15f, 0.5f),
            ClampedStr);
        RecoveryRate = 1.f / FMath::Max(RecoveryTime, 0.01f);
    }

    UFUNCTION(BlueprintPure, Category="HitReact")
    EHitDirection GetHitDirection(FVector AttackerLocation) const
    {
        ACharacter* C = Cast<ACharacter>(GetOwner());
        if (!C) return EHitDirection::Front;
        FVector ToAtk = (AttackerLocation - C->GetActorLocation()).GetSafeNormal2D();
        float F = FVector::DotProduct(C->GetActorForwardVector(), ToAtk);
        float R = FVector::DotProduct(C->GetActorRightVector(), ToAtk);
        if (F >  0.71f) return EHitDirection::Front;
        if (F < -0.71f) return EHitDirection::Back;
        if (R >  0.71f) return EHitDirection::Right;
        if (R < -0.71f) return EHitDirection::Left;
        return (F > 0) ? (R > 0 ? EHitDirection::FrontRight : EHitDirection::FrontLeft)
                       : (R > 0 ? EHitDirection::BackRight  : EHitDirection::BackLeft);
    }

protected:
    virtual void TickComponent(float Dt, ELevelTick, FActorComponentTickFunction*) override
    {
        if (!bIsRecovering) return;
        RecoveryAlpha = FMath::Clamp(RecoveryAlpha + Dt * RecoveryRate, 0.f, 1.f);

        if (USkeletalMeshComponent* Mesh = GetOwnerMesh())
        {
            float W = FMath::Lerp(HitPhysicsWeight, 1.f, RecoveryAlpha);
            Mesh->SetAllBodiesBelowPhysicsBlendWeight(ActiveHitBone, W, false, true);
        }

        if (RecoveryAlpha >= 1.f) bIsRecovering = false;
    }

private:
    USkeletalMeshComponent* GetOwnerMesh() const
    {
        ACharacter* C = Cast<ACharacter>(GetOwner());
        return C ? C->GetMesh() : nullptr;
    }
    bool   bIsRecovering = false;
    float  RecoveryAlpha = 0.f;
    float  RecoveryRate  = 1.f;
    FName  ActiveHitBone;
};
