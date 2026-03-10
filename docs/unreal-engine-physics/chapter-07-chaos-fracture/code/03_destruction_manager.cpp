// =============================================================================
// 03_destruction_manager.cpp
// 破碎管理器：预算控制/距离优先/碎片数量限制
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "DestructionManager.generated.h"

UCLASS()
class ADestructionManager : public AActor
{
    GENERATED_BODY()
public:
    static const int32 MaxActiveGC    = 12;    // 最多同时破碎中的 GC
    static const float DeactivateDist  = 3000.f; // 超过此距离停止破碎模拟

    struct FGCEntry
    {
        TWeakObjectPtr<UGeometryCollectionComponent> GCC;
        float ActivationTime = 0.f;
        float LastDistance   = 0.f;
        bool  bActive        = false;
    };

    TArray<FGCEntry> Registry;

    UFUNCTION(BlueprintCallable)
    void RegisterGC(UGeometryCollectionComponent* GCC)
    {
        FGCEntry E;
        E.GCC  = GCC;
        E.bActive = false;
        Registry.Add(E);
    }

    UFUNCTION(BlueprintCallable)
    void RequestFracture(UGeometryCollectionComponent* GCC,
        FVector FracturePoint, float Strain, float Radius)
    {
        // 统计当前活跃数
        int32 ActiveCount = 0;
        for (auto& E : Registry)
            if (E.bActive && E.GCC.IsValid()) ActiveCount++;

        if (ActiveCount >= MaxActiveGC)
        {
            // 超出预算：找到最远/最老的并降级
            FGCEntry* Oldest = nullptr;
            for (auto& E : Registry)
            {
                if (!E.bActive) continue;
                if (!Oldest || E.LastDistance > Oldest->LastDistance + 500.f
                    || E.ActivationTime < Oldest->ActivationTime)
                    Oldest = &E;
            }
            if (Oldest && Oldest->GCC.IsValid())
            {
                Oldest->GCC->SetSimulatePhysics(false);
                Oldest->bActive = false;
            }
        }

        // 激活本次破碎
        for (auto& E : Registry)
        {
            if (E.GCC.Get() == GCC)
            {
                E.bActive        = true;
                E.ActivationTime = GetWorld()->GetTimeSeconds();
                break;
            }
        }

        // 施加 Field
        AFieldSystemDemo::SpawnExplosionStrainField(
            GetWorld(), FracturePoint, Radius, Strain);
        AFieldSystemDemo::SpawnExplosionForceField(
            GetWorld(), FracturePoint, Radius * 1.5f, Strain * 0.5f);
    }

    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);
        APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
        if (!Player) return;

        for (auto& E : Registry)
        {
            if (!E.GCC.IsValid()) continue;
            float Dist = FVector::Distance(
                E.GCC->GetComponentLocation(),
                Player->GetActorLocation());
            E.LastDistance = Dist;

            // 超距离的活跃 GC：暂停物理
            if (E.bActive && Dist > DeactivateDist)
            {
                E.GCC->SetSimulatePhysics(false);
                // 不设 bActive=false，等玩家回来时重新激活
            }
            else if (E.bActive && Dist <= DeactivateDist
                && !E.GCC->IsSimulatingPhysics())
            {
                E.GCC->SetSimulatePhysics(true);
            }
        }
    }
};
