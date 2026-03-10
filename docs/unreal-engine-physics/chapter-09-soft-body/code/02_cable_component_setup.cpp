// =============================================================================
// 02_cable_component_setup.cpp
// UCableComponent 装饰绳索：多根电线/自然下垂/LOD
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/CableComponent.h"
#include "CableSetupActor.generated.h"

UCLASS()
class ACableSetupActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) TArray<FVector> AttachPoints;
    TArray<UCableComponent*> Cables;

    // 在一组附着点之间生成连接电缆（如电线杆之间的电线）
    void SpawnPowerLines()
    {
        for (int32 i = 0; i + 1 < AttachPoints.Num(); i++)
        {
            UCableComponent* Cable = NewObject<UCableComponent>(this);
            Cable->RegisterComponent();
            Cable->AttachToComponent(RootComponent,
                FAttachmentTransformRules::KeepWorldTransform);

            float Length = FVector::Distance(AttachPoints[i], AttachPoints[i+1]);
            Cable->CableLength      = Length * 1.1f;  // 略长于直线（产生自然下垂）
            Cable->NumSegments      = FMath::Clamp((int32)(Length / 80.f), 4, 16);
            Cable->SolverIterations = 4;
            Cable->CableWidth       = 1.0f;
            Cable->SubstepTime      = 0.016f;

            // 固定两端位置
            Cable->SetWorldLocation(AttachPoints[i]);
            Cable->EndLocation = AttachPoints[i+1] - AttachPoints[i]; // 相对位置
            Cable->bAttachEnd  = false; // 末端自由（或 AttachEndTo 另一个 Actor）

            // 材质
            if (WireMaterial) Cable->SetMaterial(0, WireMaterial);

            Cables.Add(Cable);
        }
    }

    // 距离优化：根据玩家距离调整段数
    UFUNCTION(BlueprintCallable)
    void UpdateCableLOD(float DistToPlayer)
    {
        int32 TargetSegments = DistToPlayer < 500.f  ? 16 :
                               DistToPlayer < 1500.f ? 8  :
                               DistToPlayer < 3000.f ? 4  : 2;
        for (UCableComponent* C : Cables)
            if (C) C->NumSegments = TargetSegments;
    }

    UPROPERTY(EditAnywhere) UMaterialInterface* WireMaterial;
};
