// =============================================================================
// 02_procedural_spine.cpp
// 程序化脊椎弯曲（AnimInstance 中计算，Control Rig 中应用）
// 对应文档：chapter-10-control-rig/07-procedural-anim.md
// =============================================================================
#pragma once
#include "Animation/AnimInstance.h"
#include "ProceduralSpineAnimInstance.generated.h"

UCLASS()
class MYPROJECT_API UProceduralSpineAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    // Control Rig 读取这些变量（通过 Get Variable 节点）
    UPROPERTY(BlueprintReadOnly) float SpineLeanAngle   = 0.f; // 前倾角度
    UPROPERTY(BlueprintReadOnly) float BreathCycle      = 0.f; // 呼吸周期值 (-1 ~ 1)
    UPROPERTY(BlueprintReadOnly) float BreathStrength   = 2.0f;// 呼吸幅度（度）
    UPROPERTY(BlueprintReadOnly) FVector LookAtPosition;       // 注视目标

    UPROPERTY(EditAnywhere, Category="ProceduralAnim")
    float BreathRate     = 0.3f;  // 呼吸频率（次/秒）
    UPROPERTY(EditAnywhere, Category="ProceduralAnim")
    float LeanScale      = 0.01f; // 速度→倾斜角映射比例
    UPROPERTY(EditAnywhere, Category="ProceduralAnim")
    float LeanInterpSpeed = 5.f;

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override
    {
        ACharacter* C = Cast<ACharacter>(TryGetPawnOwner());
        if (!C) return;

        // 1. 呼吸周期
        BreathTime  += DeltaSeconds * BreathRate;
        BreathCycle  = FMath::Sin(BreathTime * 2.f * PI);

        // 2. 根据速度动态调整呼吸幅度（奔跑后喘气）
        float Speed   = C->GetVelocity().Size2D();
        float TargetStrength = (Speed > 300.f) ? 4.0f : 2.0f;
        BreathStrength = FMath::FInterpTo(BreathStrength, TargetStrength,
                         DeltaSeconds, 1.0f);

        // 3. 脊椎前倾（基于水平速度）
        FVector ForwardVel = FVector::DotProduct(
            C->GetVelocity(), C->GetActorForwardVector()) * C->GetActorForwardVector();
        float TargetLean = ForwardVel.Size() * LeanScale;
        // 后退时反向倾斜
        if (FVector::DotProduct(C->GetVelocity(), C->GetActorForwardVector()) < 0)
            TargetLean = -TargetLean;
        SpineLeanAngle = FMath::FInterpTo(SpineLeanAngle, TargetLean,
                          DeltaSeconds, LeanInterpSpeed);

        // 4. 注视目标（如当前锁定敌人）
        if (AController* PC = C->GetController())
        {
            FVector ViewLoc; FRotator ViewRot;
            PC->GetPlayerViewPoint(ViewLoc, ViewRot);
            LookAtPosition = ViewLoc + ViewRot.Vector() * 500.f; // 前方 5m
        }
    }

private:
    float BreathTime = 0.f;
};
