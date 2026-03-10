// =============================================================================
// 03_cloth_tearing.cpp
// 布料撕裂近似实现：基于 MaxDistance 动态禁用顶点约束 + 视觉撕裂
// 注意：真实拓扑撕裂需要 Chaos Fracture，此为游戏中常用的视觉近似方案
// 对应文档：chapter-06/10-cloth-common-issues.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "ClothingSystemRuntimeInterface.h"
#include "ClothTearingDemo.generated.h"

UCLASS()
class AClothTearingActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) USkeletalMeshComponent* Mesh;

    // 撕裂参数
    UPROPERTY(EditAnywhere, Category="Tearing")
    float TearForceThreshold = 50000.f;  // 超过此力就撕裂
    UPROPERTY(EditAnywhere, Category="Tearing")
    float TearRadius = 20.f;   // 撕裂影响半径（cm）

    // ─────────────────────────────────────────────
    // 受到外力时检测撕裂
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable)
    void TryTearCloth(FVector ForcePoint, float ForceMagnitude)
    {
        if (ForceMagnitude < TearForceThreshold) return;

        // 近似方案：在撕裂点附近释放顶点约束
        // （通过 ClothingInteractor 修改 MaxDistance 参数）
        UClothingSimulationInteractor* Interactor =
            Mesh->GetClothingSimulationInteractor();
        if (!Interactor) return;

        // 在撕裂位置附近增大 MaxDistance（让顶点自由移动，模拟撕裂）
        // 实际上需要逐粒子接口，UE5.3+ 开始支持更细粒度控制
        // 目前常用替代方案：
        //   1. 关闭整个布料的长程约束（Tether）
        //   2. 在撕裂区域切换到另一套 ClothingAsset（无该区域的版本）
        
        // 方案：临时禁用长程约束（Tether Stiffness = 0）
        Interactor->SetAnimDriveSpringStiffness(0.f);
        
        // 同时：在撕裂位置施加强烈冲量（加强撕裂感）
        FVector TearImpulse = (ForcePoint - Mesh->GetComponentLocation())
            .GetSafeNormal() * ForceMagnitude * 0.5f;
        Mesh->AddImpulseAtLocation(TearImpulse, ForcePoint);

        // 播放撕裂音效/粒子
        UGameplayStatics::PlaySoundAtLocation(this, TearSound, ForcePoint);
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TearVFX, ForcePoint);

        UE_LOG(LogTemp, Warning,
            TEXT("Cloth tearing at %s, Force=%.0f"),
            *ForcePoint.ToString(), ForceMagnitude);
    }

    UPROPERTY(EditAnywhere, Category="FX") USoundBase*     TearSound;
    UPROPERTY(EditAnywhere, Category="FX") UParticleSystem* TearVFX;
};
