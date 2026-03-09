// =============================================================================
// 02_state_machine_data.cpp
// 状态机数据驱动设计（通过 DataAsset 配置状态机参数）
// 对应文档：chapter-03-anim-blueprint/04-state-machine-basics.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StateMachineData.generated.h"

/** 单个状态的配置 */
USTRUCT(BlueprintType)
struct FAnimStateConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName StateName;

    /** 最小速度阈值进入此状态 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinSpeed = 0.f;

    /** 最大速度阈值（超过则进入更快的状态）*/
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxSpeed = 9999.f;

    /** 过渡时长 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float TransitionDuration = 0.2f;
};

/** 状态机配置 DataAsset */
UCLASS(BlueprintType)
class ULocomotionStateMachineData : public UDataAsset
{
    GENERATED_BODY()

public:
    /** 地面运动状态配置 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ground")
    FAnimStateConfig IdleConfig   = { FName("Idle"),   0.f,    10.f,  0.3f };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ground")
    FAnimStateConfig WalkConfig   = { FName("Walk"),   10.f,  250.f,  0.2f };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ground")
    FAnimStateConfig RunConfig    = { FName("Run"),   250.f,  550.f,  0.2f };

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ground")
    FAnimStateConfig SprintConfig = { FName("Sprint"),550.f, 9999.f,  0.15f };

    /** 根据速度获取当前应处于的状态 */
    UFUNCTION(BlueprintPure)
    FAnimStateConfig GetStateForSpeed(float Speed) const
    {
        if (Speed >= SprintConfig.MinSpeed) return SprintConfig;
        if (Speed >= RunConfig.MinSpeed)    return RunConfig;
        if (Speed >= WalkConfig.MinSpeed)   return WalkConfig;
        return IdleConfig;
    }
};

// =============================================================================
// 状态机辅助函数集合（在 AnimInstance 中使用）
// =============================================================================
class FStateMachineUtils
{
public:
    /** 计算状态机的混合权重（用于自定义混合逻辑）*/
    static float CalcBlendAlpha(float Current, float Min, float Max)
    {
        return FMath::Clamp((Current - Min) / FMath::Max(Max - Min, 1.f), 0.f, 1.f);
    }

    /** 判断是否应该保持当前状态（防止状态抖动）*/
    static bool ShouldMaintainState(
        float Speed,
        float EnterThreshold,
        float ExitThreshold,
        bool bCurrentlyActive,
        float Hysteresis = 10.f)
    {
        // 使用迟滞区间防止速度边界附近的状态频繁切换
        if (bCurrentlyActive)
            return Speed >= (EnterThreshold - Hysteresis);
        else
            return Speed >= (EnterThreshold + Hysteresis);
    }
};
