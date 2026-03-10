// =============================================================================
// 03_kinematic_blend.cpp
// Kinematic/Physics 混合控制：移动平台、布娃娃混合、起身动画
// 对应文档：chapter-04/10-kinematic-driven.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "KinematicBlendDemo.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// 移动平台（Kinematic）
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class AKinematicPlatform : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Platform;
    UPROPERTY(EditAnywhere)    FVector StartPos;
    UPROPERTY(EditAnywhere)    FVector EndPos;
    UPROPERTY(EditAnywhere)    float   Period  = 4.f;   // 来回周期（秒）
    UPROPERTY(EditAnywhere)    float   Delay   = 0.f;   // 初始偏移
    float Elapsed = 0.f;

    AKinematicPlatform()
    {
        PrimaryActorTick.bCanEverTick = true;
        Platform = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
        Platform->SetSimulatePhysics(false);  // Kinematic！
        Platform->SetMobility(EComponentMobility::Movable);
        Platform->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Platform->SetCollisionProfileName(FName("BlockAll"));
        RootComponent = Platform;
    }

    virtual void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);
        Elapsed += DeltaTime;

        float T = (FMath::Sin((Elapsed + Delay) * 2.f * PI / Period) + 1.f) * 0.5f;
        FVector NewPos = FMath::Lerp(StartPos, EndPos, T);

        // ETeleportType::None → Chaos 根据 ΔPos/Δt 估算速度
        // → 站在平台上的物理体会感受到平台速度（被带着走）
        SetActorLocation(NewPos, false, nullptr, ETeleportType::None);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// 布娃娃混合角色
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class ARagdollBlendCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    bool  bIsRagdoll = false;
    float RagdollBlendAlpha = 0.f;
    FTimerHandle BlendTimer;
    FPoseSnapshot RagdollPoseSnapshot;

    // ── 开始变布娃娃 ──
    void StartRagdoll(float BlendDuration = 0.5f)
    {
        bIsRagdoll = true;

        // 启用骨骼物理
        GetMesh()->SetAllBodiesSimulatePhysics(true);
        GetMesh()->SetSimulatePhysics(true);
        GetCapsuleComponent()->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);

        // 逐渐混合
        RagdollBlendAlpha = 0.f;
        float StepTime = 0.033f;
        float TotalSteps = BlendDuration / StepTime;
        float StepAlpha = 1.f / TotalSteps;

        GetWorld()->GetTimerManager().SetTimer(BlendTimer,
            FTimerDelegate::CreateLambda([this, StepAlpha]()
            {
                RagdollBlendAlpha = FMath::Min(RagdollBlendAlpha + StepAlpha, 1.f);
                GetMesh()->PhysicsBlendWeight = RagdollBlendAlpha;
                if (RagdollBlendAlpha >= 1.f)
                    GetWorld()->GetTimerManager().ClearTimer(BlendTimer);
            }), StepTime, true);
    }

    // ── 开始起身 ──
    void StartGetUp()
    {
        // 1. 保存当前布娃娃姿势
        GetMesh()->SnapshotPose(RagdollPoseSnapshot);

        // 2. 关闭物理模拟
        GetMesh()->SetSimulatePhysics(false);
        GetMesh()->SetAllBodiesSimulatePhysics(false);
        GetCapsuleComponent()->SetCollisionEnabled(
            ECollisionEnabled::QueryAndPhysics);

        bIsRagdoll = false;

        // 3. 混合回动画（逐帧降低 PhysicsBlendWeight）
        RagdollBlendAlpha = 1.f;
        GetWorld()->GetTimerManager().SetTimer(BlendTimer,
            FTimerDelegate::CreateLambda([this]()
            {
                RagdollBlendAlpha = FMath::Max(RagdollBlendAlpha - 0.05f, 0.f);
                GetMesh()->PhysicsBlendWeight = RagdollBlendAlpha;
                if (RagdollBlendAlpha <= 0.f)
                    GetWorld()->GetTimerManager().ClearTimer(BlendTimer);
            }), 0.033f, true);
    }

    // ── 被击飞（Kinematic → Simulated 保持速度） ──
    void KnockBackAndRagdoll(FVector Direction, float Speed)
    {
        // 先给角色速度（Kinematic 状态）
        FBodyInstance* BI = GetMesh()->GetBodyInstance();
        if (BI)
        {
            // 设置 Kinematic 目标速度（切换为 Simulated 时保留）
            GetMesh()->SetPhysicsLinearVelocity(Direction * Speed);
        }
        StartRagdoll(0.1f);  // 快速切换到布娃娃
    }
};
