// =============================================================================
// 02_update_rate_controller.cpp
// 动画更新频率动态控制（按距离分级）
// 对应文档：chapter-08-optimization/05-update-rate.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UpdateRateController.generated.h"

UCLASS(ClassGroup=(Optimization), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UUpdateRateController : public UActorComponent
{
    GENERATED_BODY()
public:
    UUpdateRateController() { PrimaryComponentTick.bCanEverTick = true; }

    // 距离分级配置（单位 cm）
    UPROPERTY(EditAnywhere, Category="URO") float Dist_Full   = 1500.f;  // < 15m: 全速
    UPROPERTY(EditAnywhere, Category="URO") float Dist_Half   = 3000.f;  // 15~30m: 30fps
    UPROPERTY(EditAnywhere, Category="URO") float Dist_Low    = 6000.f;  // 30~60m: 15fps
    UPROPERTY(EditAnywhere, Category="URO") float Dist_Min    = 12000.f; // 60~120m: 5fps
    // > 120m: 停止更新

    UPROPERTY(EditAnywhere, Category="URO") bool bEnableInterp = true;

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        if (Mesh) Mesh->bEnableUpdateRateOptimizations = bEnableInterp;
    }

    virtual void TickComponent(float Dt, ELevelTick, FActorComponentTickFunction*) override
    {
        if (!Mesh || !PlayerPawn) return;
        UpdateCheckTimer += Dt;
        if (UpdateCheckTimer < 0.25f) return; // 每 0.25s 重新计算一次即可
        UpdateCheckTimer = 0.f;

        float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
        float Interval;
        if      (Dist < Dist_Full)  Interval = 0.f;
        else if (Dist < Dist_Half)  Interval = 1.f / 30.f;
        else if (Dist < Dist_Low)   Interval = 1.f / 15.f;
        else if (Dist < Dist_Min)   Interval = 1.f / 5.f;
        else                        Interval = 99.f; // 几乎停止

        Mesh->SetComponentTickInterval(Interval);
    }

public:
    UFUNCTION(BlueprintCallable) void SetPlayerPawn(APawn* Pawn) { PlayerPawn = Pawn; }

private:
    UPROPERTY() TObjectPtr<USkeletalMeshComponent> Mesh;
    UPROPERTY() TObjectPtr<APawn>                  PlayerPawn;
    float UpdateCheckTimer = 0.f;
};
