// =============================================================================
// 01_water_interaction.cpp
// 水体交互：入水检测/浮力/涟漪/水下效果
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WaterBodyActor.h"
#include "BuoyancyComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "WaterInteractionCharacter.generated.h"

UCLASS()
class AWaterInteractionCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="FX") UNiagaraSystem* SplashNiagara;
    UPROPERTY(EditAnywhere, Category="FX") USoundBase*     SplashSound;
    UPROPERTY(EditAnywhere, Category="FX") USoundBase*     UnderwaterAmbient;

    bool  bIsInWater    = false;
    bool  bIsUnderwater = false;
    float WaterSurfaceZ = -99999.f;
    UAudioComponent* UnderwaterAudioComp = nullptr;

    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);
        UpdateWaterState();
    }

    void UpdateWaterState()
    {
        // 查询水面高度（依赖 Water Plugin）
        float NewWaterZ = QueryWaterSurface(GetActorLocation());
        WaterSurfaceZ   = NewWaterZ;

        FVector FeetLoc = GetActorLocation();
        FVector HeadLoc = FeetLoc + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f);

        bool bFeetInWater = FeetLoc.Z < WaterSurfaceZ;
        bool bHeadInWater = HeadLoc.Z < WaterSurfaceZ;

        // ── 进出水体 ──
        if (bFeetInWater != bIsInWater)
        {
            bIsInWater = bFeetInWater;
            if (bIsInWater)  OnEnterWater();
            else             OnExitWater();
        }

        // ── 水下视觉 ──
        if (bHeadInWater != bIsUnderwater)
        {
            bIsUnderwater = bHeadInWater;
            SetUnderwaterPostProcess(bIsUnderwater);
        }

        // ── 游泳移动模式 ──
        if (bFeetInWater && GetCharacterMovement()->MovementMode != MOVE_Swimming)
            GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
        else if (!bFeetInWater && GetCharacterMovement()->MovementMode == MOVE_Swimming)
            GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    }

    void OnEnterWater()
    {
        // 入水溅水
        FVector SplashLoc(GetActorLocation().X, GetActorLocation().Y, WaterSurfaceZ);
        if (SplashNiagara)
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), SplashNiagara, SplashLoc);
        if (SplashSound)
            UGameplayStatics::PlaySoundAtLocation(this, SplashSound, SplashLoc);
    }

    void OnExitWater()
    {
        // 出水音效
        FVector SplashLoc(GetActorLocation().X, GetActorLocation().Y, WaterSurfaceZ);
        if (SplashSound)
            UGameplayStatics::PlaySoundAtLocation(this, SplashSound, SplashLoc, 0.5f);
    }

    void SetUnderwaterPostProcess(bool bUnder)
    {
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (!PC) return;
        // 通过 PlayerCameraManager 添加/移除水下 PostProcess
        // 实际实现中：用 UPostProcessComponent 挂在摄像机上
        // 在水下时 SetVisibility(true)，出水时 SetVisibility(false)
        UE_LOG(LogTemp, Log, TEXT("Underwater: %s"), bUnder ? TEXT("true") : TEXT("false"));
    }

    float QueryWaterSurface(FVector Pos)
    {
        // 简化：通过 Line Trace 找水面（或用 WaterSubsystem）
        FHitResult Hit;
        FVector Start = Pos + FVector(0, 0, 1000.f);
        FVector End   = Pos - FVector(0, 0, 100.f);
        FCollisionQueryParams P;
        P.AddIgnoredActor(this);
        // 用 Water 碰撞通道（Water Plugin 默认通道）
        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End,
            ECC_GameTraceChannel1, P))  // Water channel
            return Hit.ImpactPoint.Z;
        return -99999.f;
    }
};
