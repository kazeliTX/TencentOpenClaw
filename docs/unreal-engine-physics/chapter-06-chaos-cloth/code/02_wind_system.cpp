// =============================================================================
// 02_wind_system.cpp
// 动态风场控制系统：多区域风/阵风模拟/战场风效
// 对应文档：chapter-06/04-wind-field.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/WindDirectionalSource.h"
#include "Components/WindDirectionalSourceComponent.h"
#include "WindControlSystem.generated.h"

USTRUCT(BlueprintType)
struct FWindPreset
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) FString Name;
    UPROPERTY(EditAnywhere) float   Speed    = 100.f;
    UPROPERTY(EditAnywhere) float   Strength = 0.5f;
    UPROPERTY(EditAnywhere) float   MinGust  = 0.2f;
    UPROPERTY(EditAnywhere) float   MaxGust  = 0.8f;
    UPROPERTY(EditAnywhere) FRotator Direction = FRotator(0, 0, 0);
};

UCLASS()
class ADynamicWindSystem : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Wind") AWindDirectionalSource* WindActor;
    UPROPERTY(EditAnywhere, Category="Wind") TArray<FWindPreset> Presets;

    // 运行时状态
    FWindPreset CurrentPreset;
    FWindPreset TargetPreset;
    float TransitionTime = 0.f;
    float TransitionDuration = 2.f;
    bool  bTransitioning = false;

    // 时变分量
    float WindPhase = 0.f;
    float BaseYaw   = 0.f;
    float BaseSpeed = 100.f;

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        if (Presets.Num() > 0)
            ApplyPreset(Presets[0], 0.f);
    }

    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);
        if (bTransitioning) UpdateTransition(Dt);
        UpdateGust(Dt);
    }

    // ─────────────────────────────────────────────
    // 切换风场预设（带过渡）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable)
    void SwitchPreset(int32 PresetIndex, float Duration = 2.f)
    {
        if (!Presets.IsValidIndex(PresetIndex)) return;
        TargetPreset       = Presets[PresetIndex];
        TransitionDuration = Duration;
        TransitionTime     = 0.f;
        bTransitioning     = true;
    }

    void UpdateTransition(float Dt)
    {
        TransitionTime += Dt;
        float Alpha = FMath::Clamp(TransitionTime / TransitionDuration, 0.f, 1.f);
        float EasedAlpha = FMath::SmoothStep(0.f, 1.f, Alpha);

        FWindPreset Blended;
        Blended.Speed    = FMath::Lerp(CurrentPreset.Speed,    TargetPreset.Speed,    EasedAlpha);
        Blended.Strength = FMath::Lerp(CurrentPreset.Strength, TargetPreset.Strength, EasedAlpha);
        Blended.MinGust  = FMath::Lerp(CurrentPreset.MinGust,  TargetPreset.MinGust,  EasedAlpha);
        Blended.MaxGust  = FMath::Lerp(CurrentPreset.MaxGust,  TargetPreset.MaxGust,  EasedAlpha);
        ApplyPreset(Blended, Dt);

        if (Alpha >= 1.f)
        {
            CurrentPreset  = TargetPreset;
            bTransitioning = false;
        }
    }

    // ─────────────────────────────────────────────
    // 实时阵风（多频叠加，更自然）
    // ─────────────────────────────────────────────
    void UpdateGust(float Dt)
    {
        if (!WindActor) return;
        WindPhase += Dt;

        // 多频叠加的风速变化（模拟自然风的随机感）
        float GustFactor =
            0.5f
            + 0.3f * FMath::Sin(WindPhase * 0.7f)
            + 0.15f * FMath::Sin(WindPhase * 2.3f)
            + 0.05f * FMath::Sin(WindPhase * 5.1f);
        GustFactor = FMath::Clamp(GustFactor, CurrentPreset.MinGust, CurrentPreset.MaxGust);

        float FinalSpeed = CurrentPreset.Speed * GustFactor;

        // 风向微小摆动（±15°）
        float YawOffset = FMath::Sin(WindPhase * 0.4f) * 15.f;

        if (UWindDirectionalSourceComponent* WC = WindActor->GetComponent())
        {
            WC->Speed = FinalSpeed;
            WC->Strength = CurrentPreset.Strength * GustFactor;
        }
        WindActor->SetActorRotation(
            CurrentPreset.Direction + FRotator(0, YawOffset, 0));
    }

    void ApplyPreset(const FWindPreset& P, float Dt)
    {
        if (!WindActor) return;
        if (UWindDirectionalSourceComponent* WC = WindActor->GetComponent())
        {
            WC->Speed    = P.Speed;
            WC->Strength = P.Strength;
            WC->MinGustAmount = P.MinGust;
            WC->MaxGustAmount = P.MaxGust;
        }
        WindActor->SetActorRotation(P.Direction);
    }

    // 预设快捷键（常用场景）
    UFUNCTION(BlueprintCallable) void SetCalm()  { SwitchPreset(0, 3.f); }
    UFUNCTION(BlueprintCallable) void SetBreeze(){ SwitchPreset(1, 2.f); }
    UFUNCTION(BlueprintCallable) void SetStorm() { SwitchPreset(2, 1.f); }
};
