// =============================================================================
// 03_crowd_manager.cpp
// 简易人群动画管理器（近景骨骼动画 + 远景 ISM 切换）
// 对应文档：chapter-08-optimization/09-crowd-system.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Engine/InstancedStaticMesh.h"
#include "CrowdManager.generated.h"

UCLASS()
class MYPROJECT_API ACrowdManager : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Crowd") TSubclassOf<ACharacter> NPCClass;
    UPROPERTY(EditAnywhere, Category="Crowd") UStaticMesh*             CrowdMesh;      // VAT Mesh
    UPROPERTY(EditAnywhere, Category="Crowd") UMaterialInterface*      CrowdMaterial;  // VAT Material
    UPROPERTY(EditAnywhere, Category="Crowd") float SwitchDistance = 3000.f; // 30m 以内切换为骨骼

    void SpawnCrowdMember(FVector Location)
    {
        // 添加 ISM 实例（GPU 渲染，无 CPU 动画开销）
        int32 InstanceIdx = ISM->AddInstance(FTransform(Location));
        // 随机动画帧偏移（避免所有人同步）
        ISM->SetCustomDataValue(InstanceIdx, 0, FMath::FRandRange(0.f, 1.f));
        CrowdPositions.Add(Location);
        ActiveSkeletalNPCs.Add(nullptr); // 尚未激活骨骼版本
    }

    void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);
        if (!PlayerPawn) return;

        FVector PlayerPos = PlayerPawn->GetActorLocation();
        for (int32 i = 0; i < CrowdPositions.Num(); i++)
        {
            float Dist = FVector::Dist(CrowdPositions[i], PlayerPos);
            bool bNeedsSkeletalNPC = (Dist < SwitchDistance);
            bool bHasSkelNPC = (ActiveSkeletalNPCs[i] != nullptr);

            if (bNeedsSkeletalNPC && !bHasSkelNPC)
            {
                // 切换为骨骼 NPC
                FActorSpawnParameters Params;
                ACharacter* NPC = GetWorld()->SpawnActor<ACharacter>(
                    NPCClass, CrowdPositions[i], FRotator::ZeroRotator, Params);
                ActiveSkeletalNPCs[i] = NPC;
                // 隐藏对应 ISM 实例
                ISM->UpdateInstanceTransform(i,
                    FTransform(FVector(CrowdPositions[i] + FVector(0,0,-10000))), true);
            }
            else if (!bNeedsSkeletalNPC && bHasSkelNPC)
            {
                // 切回 ISM
                ActiveSkeletalNPCs[i]->Destroy();
                ActiveSkeletalNPCs[i] = nullptr;
                ISM->UpdateInstanceTransform(i, FTransform(CrowdPositions[i]), true);
            }
        }
    }

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        ISM = NewObject<UInstancedStaticMeshComponent>(this);
        ISM->SetStaticMesh(CrowdMesh);
        ISM->SetMaterial(0, CrowdMaterial);
        ISM->RegisterComponent();
        PlayerPawn = GetWorld()->GetFirstPlayerController()
            ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    }

private:
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> ISM;
    UPROPERTY() TObjectPtr<APawn>                         PlayerPawn;
    TArray<FVector>     CrowdPositions;
    TArray<ACharacter*> ActiveSkeletalNPCs;
};
