// =============================================================================
// 02_physics_anim.cpp
// 物理动画混合示例
// 演示：PhysicalAnimationComponent 配置、受击物理反应
// 对应文档：chapter-02-skeleton-system/07-ragdoll-system.md
// =============================================================================

#pragma once
#include "CoreMinimal.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "GameFramework/Character.h"

/**
 * 物理动画混合组件示例
 * 实现：上半身受击时有物理弹性，同时下半身继续播放运动动画
 */
class FPhysicsAnimSetup
{
public:
    static void SetupPhysicalAnimation(
        ACharacter* Character,
        UPhysicalAnimationComponent* PhysAnim)
    {
        if (!Character || !PhysAnim) return;

        USkeletalMeshComponent* Mesh = Character->GetMesh();

        // 绑定目标 Mesh
        PhysAnim->SetSkeletalMeshComponent(Mesh);

        // 配置物理动画参数
        FPhysicalAnimationData AnimData;
        AnimData.bIsLocalSimulation       = false;  // 世界空间（更稳定）
        AnimData.OrientationStrength      = 1000.0f; // 骨骼朝向跟随动画的强度
        AnimData.AngularVelocityStrength  = 100.0f;  // 角速度阻尼
        AnimData.PositionStrength         = 1000.0f; // 骨骼位置跟随动画的强度
        AnimData.VelocityStrength         = 100.0f;  // 速度阻尼
        AnimData.MaxLinearForce           = 0.0f;    // 0 = 无限制
        AnimData.MaxAngularForce          = 0.0f;

        // 对上半身（spine_03 以上）应用物理动画
        PhysAnim->ApplyPhysicalAnimationSettingsBelow(
            FName("spine_03"),
            AnimData,
            true   // 包含 spine_03 自身
        );

        // 开启上半身物理模拟（让骨骼有物理弹性）
        Mesh->SetAllBodiesBelowSimulatePhysics(
            FName("spine_03"),
            true,  // 开启物理
            true   // 递归子骨骼
        );

        // 设置混合权重（0.0 = 纯动画，1.0 = 纯物理）
        // Physical Animation 自动在动画和物理之间混合
        // 通过 Spring Strength 参数控制偏向
    }

    static void ApplyHitReaction(
        ACharacter* Character,
        FName HitBone,
        FVector ImpulseDir,
        float ImpulseStrength = 30000.0f)
    {
        if (!Character) return;
        USkeletalMeshComponent* Mesh = Character->GetMesh();

        // 施加冲量到被击中的骨骼
        FVector Impulse = ImpulseDir.GetSafeNormal() * ImpulseStrength;
        Mesh->AddImpulse(Impulse, HitBone, true);  // true = velocity change
    }

    static void ResetToAnimation(
        ACharacter* Character,
        UPhysicalAnimationComponent* PhysAnim)
    {
        if (!Character) return;
        USkeletalMeshComponent* Mesh = Character->GetMesh();

        // 关闭上半身物理，恢复纯动画控制
        Mesh->SetAllBodiesBelowSimulatePhysics(
            FName("spine_03"),
            false,
            true
        );
    }
};

// =============================================================================
// 使用示例（在 AMyCharacter 中）：
//
// UPROPERTY(VisibleAnywhere)
// UPhysicalAnimationComponent* PhysAnim;
//
// void AMyCharacter::BeginPlay()
// {
//     Super::BeginPlay();
//     PhysAnim = FindComponentByClass<UPhysicalAnimationComponent>();
//     FPhysicsAnimSetup::SetupPhysicalAnimation(this, PhysAnim);
// }
//
// void AMyCharacter::OnTakeHit(FVector HitDir, FName HitBone)
// {
//     FPhysicsAnimSetup::ApplyHitReaction(this, HitBone, HitDir);
//
//     // 1.5 秒后恢复
//     GetWorldTimerManager().SetTimer(RecoverTimer, [this]()
//     {
//         FPhysicsAnimSetup::ResetToAnimation(this, PhysAnim);
//     }, 1.5f, false);
// }
// =============================================================================
