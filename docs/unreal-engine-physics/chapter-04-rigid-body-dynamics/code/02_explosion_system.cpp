// =============================================================================
// 02_explosion_system.cpp
// 完整爆炸系统：自定义径向冲量、破片生成、屏幕震动
// 对应文档：chapter-04/04-explosion-force.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "ExplosionSystem.generated.h"

USTRUCT(BlueprintType)
struct FExplosionParams
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) float Radius         = 500.f;
    UPROPERTY(EditAnywhere) float InnerRadius    = 100.f;  // 全力内圈
    UPROPERTY(EditAnywhere) float BaseImpulse    = 200000.f;
    UPROPERTY(EditAnywhere) float VerticalBoost  = 0.4f;   // 上抛系数
    UPROPERTY(EditAnywhere) float Damage         = 100.f;
    UPROPERTY(EditAnywhere) float DamageRadius   = 400.f;
    UPROPERTY(EditAnywhere) int32 NumDebris      = 8;
    UPROPERTY(EditAnywhere) float DebrisLifespan = 5.f;
    UPROPERTY(EditAnywhere) float CameraShakeRadius = 1000.f;
};

UCLASS()
class AExplosionActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Explosion") FExplosionParams Params;
    UPROPERTY(EditAnywhere, Category="FX")  UParticleSystem*       ExplosionVFX;
    UPROPERTY(EditAnywhere, Category="FX")  USoundBase*            ExplosionSFX;
    UPROPERTY(EditAnywhere, Category="FX")  TSubclassOf<UCameraShakeBase> ShakeClass;
    UPROPERTY(EditAnywhere, Category="Debris") TSubclassOf<AActor> DebrisClass;

    UFUNCTION(BlueprintCallable, Category="Explosion")
    void Explode()
    {
        FVector Center = GetActorLocation();

        // 1. 视觉/音效
        if (ExplosionVFX)
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVFX, Center);
        if (ExplosionSFX)
            UGameplayStatics::PlaySoundAtLocation(this, ExplosionSFX, Center);

        // 2. 径向物理冲量
        ApplyExplosionPhysics(Center);

        // 3. 造成伤害
        UGameplayStatics::ApplyRadialDamage(
            GetWorld(), Params.Damage, Center, Params.DamageRadius,
            nullptr, {}, this, nullptr, true);

        // 4. 相机震动
        if (ShakeClass)
            UGameplayStatics::PlayWorldCameraShake(
                GetWorld(), ShakeClass, Center,
                0.f, Params.CameraShakeRadius, 1.f);

        // 5. 生成破片
        SpawnDebris(Center);

        // 6. 销毁自身
        SetLifeSpan(0.1f);
    }

private:
    void ApplyExplosionPhysics(FVector Center)
    {
        TArray<FOverlapResult> Overlaps;
        FCollisionObjectQueryParams ObjParams;
        ObjParams.AddObjectTypesToQuery(ECC_PhysicsBody);
        ObjParams.AddObjectTypesToQuery(ECC_Destructible);
        ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);

        GetWorld()->OverlapMultiByObjectType(
            Overlaps, Center, FQuat::Identity, ObjParams,
            FCollisionShape::MakeSphere(Params.Radius));

        TSet<UPrimitiveComponent*> Processed;
        for (const FOverlapResult& R : Overlaps)
        {
            UPrimitiveComponent* Comp = R.Component.Get();
            if (!Comp || Processed.Contains(Comp)) continue;
            if (!Comp->IsSimulatingPhysics()) continue;
            Processed.Add(Comp);

            FVector CompCenter = Comp->GetCenterOfMass();
            float   Dist       = FVector::Distance(Center, CompCenter);
            if (Dist < 1.f) Dist = 1.f;

            // 衰减：内圈全力，外圈线性衰减
            float FallOff;
            if (Dist <= Params.InnerRadius)
                FallOff = 1.f;
            else
                FallOff = FMath::Max(0.f,
                    1.f - (Dist - Params.InnerRadius)
                        / (Params.Radius - Params.InnerRadius));

            if (FallOff <= 0.f) continue;

            // 方向：径向 + 向上分量
            FVector Dir = (CompCenter - Center).GetSafeNormal();
            Dir.Z += Params.VerticalBoost;
            Dir.Normalize();

            float ImpulseMag = Params.BaseImpulse * FallOff;
            Comp->AddImpulseAtLocation(Dir * ImpulseMag, Center);
        }
    }

    void SpawnDebris(FVector Center)
    {
        if (!DebrisClass) return;
        for (int32 i = 0; i < Params.NumDebris; ++i)
        {
            FVector Offset = FMath::VRand() * 30.f;
            FRotator Rot   = FRotator(
                FMath::RandRange(-180.f, 180.f),
                FMath::RandRange(-180.f, 180.f), 0);

            AActor* D = GetWorld()->SpawnActor<AActor>(
                DebrisClass, Center + Offset, Rot);
            if (!D) continue;

            UPrimitiveComponent* DC =
                D->FindComponentByClass<UPrimitiveComponent>();
            if (DC && DC->IsSimulatingPhysics())
            {
                // 随机飞散方向（确保向上分量）
                FVector DebrisDir = FMath::VRand();
                DebrisDir.Z = FMath::Abs(DebrisDir.Z) + 0.3f;
                DebrisDir.Normalize();
                float Mag = FMath::RandRange(50000.f, 150000.f);
                DC->AddImpulse(DebrisDir * Mag, NAME_None, false);

                FVector AngImpulse = FMath::VRand() *
                    FMath::RandRange(200.f, 800.f);
                DC->AddAngularImpulseInDegrees(AngImpulse);
            }
            D->SetLifeSpan(Params.DebrisLifespan);
        }
    }
};
