// =============================================================================
// 03_soft_body_demo.cpp
// 软体近似：Chaos Cloth 作为软包 / 顶点动画果冻
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "SoftBodyDemo.generated.h"

UCLASS()
class ASoftBodyActor : public AActor
{
    GENERATED_BODY()
public:
    // Chaos Cloth 实现的软包（使用 SkeletalMesh + ClothingAsset）
    UPROPERTY(VisibleAnywhere) USkeletalMeshComponent* SoftMesh;

    // 受击时的果冻反应
    UFUNCTION(BlueprintCallable)
    void OnHit(FVector HitPoint, FVector HitNormal, float Force)
    {
        // 在命中点施加冲量（Cloth 粒子会响应）
        SoftMesh->AddImpulseAtLocation(
            HitNormal * -Force, HitPoint);

        // 重置布料（防止被打飞）
        if (Force > 5000.f)
            SoftMesh->ForceClothNextUpdateTeleportReset();
    }

    // 挤压效果（上方向下挤）
    UFUNCTION(BlueprintCallable)
    void Squeeze(float SqueezeAmount)
    {
        // 通过 SetRelativeScale3D 模拟挤压（视觉近似）
        // 真实体积保持需要 Chaos Soft Body
        float ScaleXY = 1.f + SqueezeAmount * 0.3f;
        float ScaleZ  = 1.f - SqueezeAmount * 0.3f;
        SoftMesh->SetRelativeScale3D(FVector(ScaleXY, ScaleXY, ScaleZ));

        // 延迟回弹
        FTimerHandle T;
        GetWorld()->GetTimerManager().SetTimer(T,
            FTimerDelegate::CreateWeakLambda(this, [this]() {
                SoftMesh->SetRelativeScale3D(FVector::OneVector);
                // 可以加弹簧插值，这里仅示意
            }), 0.3f, false);
    }
};
