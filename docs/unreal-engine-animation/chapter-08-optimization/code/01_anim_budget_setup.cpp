// =============================================================================
// 01_anim_budget_setup.cpp
// Animation Budget Allocator 配置与 Significance Manager 注册
// 对应文档：chapter-08-optimization/04-animation-budget.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SignificanceManager.h"
#include "AnimBudgetSetupComponent.generated.h"

UCLASS(ClassGroup=(Optimization), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UAnimBudgetSetupComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Budget") float BaseSignificance   = 1.0f;
    UPROPERTY(EditAnywhere, Category="Budget") bool  bHighPriority      = false;
    UPROPERTY(EditAnywhere, Category="Budget") float ForceHighPrioDist  = 10.f * 100.f;

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        if (USignificanceManager* SM = FSignificanceManagerModule::Get(GetWorld()))
        {
            SM->RegisterObject(GetOwner(), FName("Character"),
                [this](USignificanceManager::FManagedObjectInfo* Info,
                       const FTransform& ViewTF) -> float
                {
                    AActor* Owner = Cast<AActor>(Info->GetObject());
                    if (!Owner) return 0.f;
                    float Dist = FVector::Dist(Owner->GetActorLocation(), ViewTF.GetLocation());
                    // 强制高优先级角色（BOSS等）始终高权重
                    if (bHighPriority && Dist < ForceHighPrioDist) return 10.f;
                    // 距离越近权重越高（Budget Allocator 优先更新）
                    return BaseSignificance * FMath::Clamp(1000.f / FMath::Max(Dist, 1.f), 0.f, 1.f);
                });
        }
    }

    virtual void EndPlay(const EEndPlayReason::Type Reason) override
    {
        Super::EndPlay(Reason);
        if (USignificanceManager* SM = FSignificanceManagerModule::Get(GetWorld()))
            SM->UnregisterObject(GetOwner());
    }
};
