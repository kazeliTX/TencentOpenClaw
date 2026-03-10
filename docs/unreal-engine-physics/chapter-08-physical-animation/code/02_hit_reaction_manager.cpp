// =============================================================================
// 02_hit_reaction_manager.cpp
// 命中反应管理器：多部位命中叠加/累积伤害/布娃娃阈值
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "HitReactionManager.generated.h"

USTRUCT(BlueprintType)
struct FHitAccumulator
{
    GENERATED_BODY()
    float   AccumulatedImpulse = 0.f;
    float   LastHitTime        = -999.f;
    int32   HitCount           = 0;
    float   ResetInterval      = 1.0f;   // 1秒无命中则重置

    void AddHit(float Impulse, float Now)
    {
        if (Now - LastHitTime > ResetInterval)
        {
            AccumulatedImpulse = 0.f;
            HitCount = 0;
        }
        AccumulatedImpulse += Impulse;
        HitCount++;
        LastHitTime = Now;
    }
};

UCLASS()
class AHitReactionCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UPhysicalAnimationComponent* PhysAnim;
    
    // 各骨骼区域的累积命中记录
    TMap<FName, FHitAccumulator> HitAccumulators;
    
    UPROPERTY(EditAnywhere) float RagdollThreshold       = 5000.f;
    UPROPERTY(EditAnywhere) float StrongHitImpulse       = 800.f;

    UFUNCTION(BlueprintCallable)
    void ProcessHit(const FHitResult& Hit, float Damage)
    {
        FName Bone = Hit.BoneName;
        float Now  = GetWorld()->GetTimeSeconds();
        float Impulse = Damage * 15.f; // damage → impulse 换算

        // 记录累积
        FHitAccumulator& Acc = HitAccumulators.FindOrAdd(Bone);
        Acc.AddHit(Impulse, Now);

        // 施加冲量
        GetMesh()->AddImpulseAtLocation(
            -Hit.ImpactNormal * Impulse,
            Hit.ImpactPoint, Bone);

        // 根据累积判断反应级别
        if (Acc.AccumulatedImpulse > RagdollThreshold)
        {
            TriggerRagdoll();
        }
        else if (Impulse > StrongHitImpulse)
        {
            // 强命中：更低的追随强度，更长的恢复时间
            PhysAnim->SetStrengthMultiplier(0.05f, Bone);
            SetRestoreTimer(Bone, 0.8f);
        }
        else
        {
            // 普通命中
            PhysAnim->SetStrengthMultiplier(0.2f, Bone);
            SetRestoreTimer(Bone, 0.4f);
        }
    }

    void SetRestoreTimer(FName Bone, float Delay)
    {
        FTimerHandle T;
        GetWorld()->GetTimerManager().SetTimer(T,
            FTimerDelegate::CreateWeakLambda(this,
                [this, Bone]() {
                    PhysAnim->SetStrengthMultiplier(1.0f, Bone);
                }), Delay, false);
    }

    void TriggerRagdoll()
    {
        GetCharacterMovement()->DisableMovement();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetMesh()->SetAllBodiesSimulatePhysics(true);
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    }
};
