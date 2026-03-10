// =============================================================================
// 03_niagara_fluid_controller.cpp
// Niagara Fluid 控制器：外力注入/分辨率切换/距离管理
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraFluidController.generated.h"

UCLASS()
class ANiagaraFluidController : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UNiagaraComponent* FluidComp;
    UPROPERTY(EditAnywhere)    float MaxInteractDist = 500.f;
    bool bSimActive = true;

    // ── 在水面位置产生涟漪 ──
    UFUNCTION(BlueprintCallable)
    void Splash(FVector WorldPos, float Strength = 1.0f)
    {
        FVector LocalPos = FluidComp->GetComponentTransform()
            .InverseTransformPosition(WorldPos);
        // 归一化到 [-0.5, 0.5]
        FVector GridExtent = FVector(FluidComp->Bounds.BoxExtent);
        LocalPos /= GridExtent;

        FluidComp->SetNiagaraVariableVec3(
            TEXT("User.SplashPosition"), LocalPos);
        FluidComp->SetNiagaraVariableFloat(
            TEXT("User.SplashStrength"), Strength);
        FluidComp->SetNiagaraVariableBool(
            TEXT("User.bTriggerSplash"), true);

        // 单帧触发后重置
        FTimerHandle T;
        GetWorld()->GetTimerManager().SetTimer(T,
            FTimerDelegate::CreateWeakLambda(this, [this]() {
                if (FluidComp)
                    FluidComp->SetNiagaraVariableBool(
                        TEXT("User.bTriggerSplash"), false);
            }), 0.05f, false);
    }

    // ── 根据玩家距离管理模拟开/关 ──
    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);

        APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
        if (!Player) return;

        float Dist = FVector::Distance(
            GetActorLocation(), Player->GetActorLocation());

        bool bShouldBeActive = (Dist < MaxInteractDist);
        if (bShouldBeActive != bSimActive)
        {
            bSimActive = bShouldBeActive;
            if (bSimActive)
                FluidComp->ActivateSystem();
            else
                FluidComp->DeactivateImmediate();
        }
    }
};
