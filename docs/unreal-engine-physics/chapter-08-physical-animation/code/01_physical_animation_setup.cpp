// =============================================================================
// 01_physical_animation_setup.cpp
// 物理动画组件完整配置：命中反应/逐骨骼参数/强度控制
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "PhysicalAnimCharacter.generated.h"

UCLASS()
class APhysicalAnimCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UPhysicalAnimationComponent* PhysAnim;

    APhysicalAnimCharacter()
    {
        PhysAnim = CreateDefaultSubobject<UPhysicalAnimationComponent>(
            TEXT("PhysAnim"));
    }

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        PhysAnim->SetSkeletalMeshComponent(GetMesh());
        
        // 应用 Profile（从 spine_01 以下）
        PhysAnim->ApplyPhysicalAnimationProfileBelow(
            FName("spine_01"), FName("HitReact"), true, true);
        
        // 开启骨骼物理（必须！）
        GetMesh()->SetAllBodiesBelowSimulatePhysics(
            FName("spine_01"), true, true);
        GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(
            FName("spine_01"), 1.0f, true);
    }

    // ─── 命中反应 ───
    UFUNCTION(BlueprintCallable)
    void HandleHit(FVector ImpactPoint, FVector ImpactNormal,
        float ImpulseMag, FName BoneName)
    {
        GetMesh()->AddImpulseAtLocation(
            -ImpactNormal * ImpulseMag, ImpactPoint, BoneName);
        
        // 命中瞬间降低追随强度（让物理反应明显）
        PhysAnim->SetStrengthMultiplier(0.15f, BoneName);
        
        FTimerHandle T;
        float BoneName_f = 0.f; // 仅用于 lambda 捕获
        GetWorld()->GetTimerManager().SetTimer(T,
            FTimerDelegate::CreateWeakLambda(this,
                [this, BoneName]() {
                    PhysAnim->SetStrengthMultiplier(1.0f, BoneName);
                }), 0.5f, false);
    }

    // ─── 进水/出水切换 ───
    UFUNCTION(BlueprintCallable)
    void SetUnderwater(bool bUnder)
    {
        // 水中：弱追随（漂浮感）
        float S = bUnder ? 0.05f : 1.0f;
        PhysAnim->SetStrengthMultiplier(S);
        
        if (bUnder)
        {
            // 降低角速度阻尼（水中更飘）
            FPhysicalAnimationData D;
            D.bIsLocalSimulation      = true;
            D.OrientationStrength     = 100.f;
            D.AngularVelocityStrength = 5.f;
            // 对所有骨骼应用（仅示意，实际逐骨骼调用）
        }
    }
};
