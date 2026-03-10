// =============================================================================
// 02_partial_ragdoll.cpp
// 部分布娃娃：上半身受击反应 + 逐骨骼物理权重
// 对应文档：chapter-05/07-hit-reaction.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Curves/CurveFloat.h"
#include "PartialRagdollCharacter.generated.h"

UCLASS()
class APartialRagdollCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    // 受击恢复曲线（1.0 → 0.0，约 1.5s）
    UPROPERTY(EditAnywhere, Category="HitReaction") UCurveFloat* HitRecoverCurve;

    bool  bHitReacting = false;
    float HitReactTime = 0.f;
    FName HitBoneName;

    // 命中部位 → 冲量倍率
    static const TMap<FName, float>& GetHitMultiplierMap()
    {
        static TMap<FName, float> Map = {
            { "head",       5.0f }, { "neck_01",    4.0f },
            { "spine_03",   3.0f }, { "spine_02",   2.5f },
            { "spine_01",   2.0f }, { "pelvis",     2.0f },
            { "upperarm_l", 1.5f }, { "upperarm_r", 1.5f },
            { "thigh_l",    1.5f }, { "thigh_r",    1.5f },
            { "lowerarm_l", 1.0f }, { "lowerarm_r", 1.0f },
            { "calf_l",     1.0f }, { "calf_r",     1.0f },
        };
        return Map;
    }

    // ─────────────────────────────────────────────
    // 受击处理
    // ─────────────────────────────────────────────
    void HandleBulletHit(const FHitResult& Hit, float BulletImpulse)
    {
        HitBoneName = Hit.BoneName;
        const float* Mult = GetHitMultiplierMap().Find(HitBoneName);
        float ImpulseMag  = BulletImpulse * (Mult ? *Mult : 1.0f);

        // 施加冲量（命中点，命中方向）
        GetMesh()->AddImpulseAtLocation(
            -Hit.ImpactNormal * ImpulseMag,
            Hit.ImpactPoint,
            HitBoneName);

        // 开启部分物理
        ActivatePartialPhysics(HitBoneName);
    }

    // ─────────────────────────────────────────────
    // 激活以命中骨骼为根的部分布娃娃
    // ─────────────────────────────────────────────
    void ActivatePartialPhysics(FName RootBone)
    {
        if (bHitReacting) return;  // 防止重入
        bHitReacting   = true;
        HitReactTime   = 0.f;

        // 该骨骼以下（含）开启物理
        GetMesh()->SetAllBodiesBelowSimulatePhysics(RootBone, true, true);
        GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(RootBone, 1.0f, true);
    }

    virtual void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);

        if (!bHitReacting || !HitRecoverCurve) return;

        HitReactTime += DeltaTime;
        float Weight  = HitRecoverCurve->GetFloatValue(HitReactTime);
        float Clamped = FMath::Clamp(Weight, 0.f, 1.f);

        // 逐步降低物理权重
        GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(
            HitBoneName, Clamped, true);

        // 最后一帧关闭物理
        const FRealCurve* Curve = HitRecoverCurve->FloatCurve.GetRichCurveConst();
        if (Curve && HitReactTime >=
            Curve->GetKeyTime(Curve->GetLastKeyHandle()))
        {
            GetMesh()->SetAllBodiesBelowSimulatePhysics(
                HitBoneName, false, true);
            GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(
                HitBoneName, 0.f, true);
            bHitReacting = false;
        }
    }
};
