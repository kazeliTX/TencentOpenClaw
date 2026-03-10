// =============================================================================
// 02_vehicle_drift.cpp
// 漂移系统：手刹触发/漂移检测/烟雾特效/计分
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "NiagaraFunctionLibrary.h"
#include "VehicleDriftSystem.generated.h"

UCLASS()
class AVehicleDriftSystem : public AMyVehicle
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) UNiagaraSystem* TireSmoke;
    
    bool   bIsDrifting     = false;
    float  DriftScore      = 0.f;
    float  DriftAngle      = 0.f;   // 车头朝向 vs 速度方向的夹角

    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);
        UpdateDriftState(Dt);
    }

    void UpdateDriftState(float Dt)
    {
        FVector Velocity   = GetVelocity();
        FVector ForwardDir = GetActorForwardVector();
        float   Speed      = Velocity.Size();

        if (Speed < 100.f) { bIsDrifting = false; return; }

        // 计算漂移角（速度方向与车头方向的夹角）
        FVector VelDir = Velocity.GetSafeNormal();
        float   Cos    = FVector::DotProduct(ForwardDir, VelDir);
        DriftAngle     = FMath::RadiansToDegrees(FMath::Acos(
            FMath::Clamp(Cos, -1.f, 1.f)));

        bool bNewDrift = (DriftAngle > 15.f && Speed > 500.f);

        if (bNewDrift != bIsDrifting)
        {
            bIsDrifting = bNewDrift;
            if (bIsDrifting)  OnDriftStart();
            else              OnDriftEnd();
        }

        if (bIsDrifting)
        {
            // 积分漂移分数
            DriftScore += DriftAngle * Speed * Dt * 0.0001f;
            SpawnTireSmoke();
        }
    }

    void OnDriftStart()
    {
        UE_LOG(LogTemp, Log, TEXT("Drift started!"));
    }

    void OnDriftEnd()
    {
        UE_LOG(LogTemp, Log, TEXT("Drift ended! Score: %.1f"), DriftScore);
        DriftScore = 0.f;
    }

    void SpawnTireSmoke()
    {
        if (!TireSmoke) return;
        // 在后轮位置生成烟雾
        for (FName BoneName : { FName("wheel_RL"), FName("wheel_RR") })
        {
            FVector WheelLoc = GetMesh()->GetSocketLocation(BoneName);
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), TireSmoke, WheelLoc,
                FRotator::ZeroRotator, FVector(0.5f), true, true,
                ENCPoolMethod::AutoRelease);
        }
    }
};
