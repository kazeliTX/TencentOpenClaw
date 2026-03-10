// =============================================================================
// 01_cloth_runtime_control.cpp
// 运行时布料控制：传送重置/暂停恢复/参数切换/水下效果
// 对应文档：chapter-06/09-runtime-cloth-control.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "ClothingSystemRuntimeInterface.h"
#include "ClothRuntimeControlDemo.generated.h"

UCLASS()
class AClothCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    FVector LastLocation;
    float   TeleportSpeedThreshold = 1000.f; // cm/s
    bool    bIsUnderwater = false;

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        LastLocation = GetActorLocation();
    }

    virtual void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);
        HandleClothTeleport(DeltaTime);
    }

    // ─────────────────────────────────────────────
    // 自动检测高速运动并触发 Teleport
    // ─────────────────────────────────────────────
    void HandleClothTeleport(float Dt)
    {
        FVector Delta = GetActorLocation() - LastLocation;
        float Speed   = Delta.Size() / FMath::Max(Dt, 0.001f);

        if (Speed > TeleportSpeedThreshold)
        {
            // 高速移动：Teleport（保持姿势，修正位置）
            GetMesh()->ForceClothNextUpdateTeleport();
        }
        LastLocation = GetActorLocation();
    }

    // ─────────────────────────────────────────────
    // 传送角色（必须重置布料）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable)
    void TeleportTo(FVector TargetLocation)
    {
        // ❶ 先重置布料（在下一帧物理更新时生效）
        GetMesh()->ForceClothNextUpdateTeleportReset();
        // ❷ 移动位置
        SetActorLocation(TargetLocation, false, nullptr,
            ETeleportType::TeleportPhysics);
    }

    // ─────────────────────────────────────────────
    // 进入/离开水下
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable)
    void SetUnderwater(bool bEnter)
    {
        bIsUnderwater = bEnter;
        UClothingSimulationInteractor* Interactor =
            GetMesh()->GetClothingSimulationInteractor();
        if (!Interactor) return;

        if (bEnter)
        {
            // 水下：增大阻尼，降低重力（浮力近似）
            Interactor->SetAnimDriveSpringStiffness(0.05f);
            // 通过修改物理体参数调整阻尼（间接方式）
            UE_LOG(LogTemp, Log, TEXT("Entering underwater: cloth damping increased"));
        }
        else
        {
            Interactor->SetAnimDriveSpringStiffness(0.0f);
            UE_LOG(LogTemp, Log, TEXT("Exiting underwater: cloth damping restored"));
        }
    }

    // ─────────────────────────────────────────────
    // 暂停布料（优化远距离/不可见角色）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable)
    void SetClothEnabled(bool bEnable)
    {
        if (bEnable)
            GetMesh()->ResumeClothingSimulation();
        else
            GetMesh()->SuspendClothingSimulation();
    }

    // ─────────────────────────────────────────────
    // 基于距离的布料质量控制
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable)
    void UpdateClothQualityByDistance(float DistToPlayer)
    {
        if (DistToPlayer > 3000.f)
        {
            GetMesh()->SuspendClothingSimulation();
        }
        else if (DistToPlayer > 1000.f)
        {
            GetMesh()->ResumeClothingSimulation();
            // 低质量模式：减少迭代（通过 CVar）
            static IConsoleVariable* IterCV =
                IConsoleManager::Get().FindConsoleVariable(
                    TEXT("p.ChaosCloth.MaxNumIterations"));
            if (IterCV) IterCV->Set(2);
        }
        else
        {
            GetMesh()->ResumeClothingSimulation();
            static IConsoleVariable* IterCV =
                IConsoleManager::Get().FindConsoleVariable(
                    TEXT("p.ChaosCloth.MaxNumIterations"));
            if (IterCV) IterCV->Set(8);
        }
    }
};
