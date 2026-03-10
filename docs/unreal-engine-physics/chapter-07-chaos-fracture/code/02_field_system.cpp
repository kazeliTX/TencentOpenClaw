// =============================================================================
// 02_field_system.cpp
// Field System 完整示例：爆炸应变/径向力/睡眠激活/锚定解除
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Field/FieldSystemActor.h"
#include "Field/FieldSystemComponent.h"
#include "FieldSystemDemo.generated.h"

UCLASS()
class AFieldSystemDemo : public AActor
{
    GENERATED_BODY()
public:
    // ─ 爆炸 + 应变场（触发破碎）─
    UFUNCTION(BlueprintCallable)
    static void SpawnExplosionStrainField(UWorld* W,
        FVector Center, float Radius, float Strain)
    {
        AFieldSystemActor* FA = W->SpawnActor<AFieldSystemActor>(
            AFieldSystemActor::StaticClass(), Center, FRotator::ZeroRotator);
        if (!FA) return;

        UFieldSystemComponent* FSC = FA->GetFieldSystemComponent();
        FSC->ApplyLinearForce(false,
            EFieldPhysicsType::Field_ExternalClusterStrain,
            FFieldSystemMetaDataFilter(),
            Center, Strain, Radius,
            EFieldFalloffType::Field_FallOff_Linear);
        FA->SetLifeSpan(0.05f);
    }

    // ─ 径向爆炸力（让碎片飞散）─
    UFUNCTION(BlueprintCallable)
    static void SpawnExplosionForceField(UWorld* W,
        FVector Center, float Radius, float ForceMag)
    {
        AFieldSystemActor* FA = W->SpawnActor<AFieldSystemActor>(
            AFieldSystemActor::StaticClass(), Center, FRotator::ZeroRotator);
        if (!FA) return;

        UFieldSystemComponent* FSC = FA->GetFieldSystemComponent();
        // 径向力：从 Center 向外
        FSC->ApplyLinearForce(false,
            EFieldPhysicsType::Field_LinearForce,
            FFieldSystemMetaDataFilter(),
            Center, ForceMag, Radius,
            EFieldFalloffType::Field_FallOff_Linear);
        FA->SetLifeSpan(0.1f);
    }

    // ─ 睡眠→激活场（唤醒碎片）─
    UFUNCTION(BlueprintCallable)
    static void SpawnDynamicStateField(UWorld* W,
        FVector Center, float Radius)
    {
        AFieldSystemActor* FA = W->SpawnActor<AFieldSystemActor>(
            AFieldSystemActor::StaticClass(), Center, FRotator::ZeroRotator);
        if (!FA) return;

        UFieldSystemComponent* FSC = FA->GetFieldSystemComponent();
        // 将范围内静止碎片激活
        FSC->ApplyLinearForce(false,
            EFieldPhysicsType::Field_DynamicConstraint,
            FFieldSystemMetaDataFilter(),
            Center, 1.0f, Radius,
            EFieldFalloffType::Field_FallOff_None);
        FA->SetLifeSpan(0.05f);
    }

    // ─ 重力场（局部反重力，碎片漂浮效果）─
    UFUNCTION(BlueprintCallable)
    static void SpawnAntiGravityField(UWorld* W,
        FVector Center, float Radius, float LiftStrength)
    {
        AFieldSystemActor* FA = W->SpawnActor<AFieldSystemActor>(
            AFieldSystemActor::StaticClass(), Center, FRotator::ZeroRotator);
        if (!FA) return;

        UFieldSystemComponent* FSC = FA->GetFieldSystemComponent();
        FSC->ApplyLinearForce(false,
            EFieldPhysicsType::Field_LinearForce,
            FFieldSystemMetaDataFilter(),
            Center + FVector(0,0,1) * LiftStrength,
            LiftStrength, Radius,
            EFieldFalloffType::Field_FallOff_Linear);
        FA->SetLifeSpan(0.05f);  // 持续施加需在 Tick 中重复生成
    }
};
