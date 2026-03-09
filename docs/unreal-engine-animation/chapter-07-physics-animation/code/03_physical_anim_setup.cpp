// =============================================================================
// 03_physical_anim_setup.cpp
// Physical Animation Component 配置辅助
// 对应文档：chapter-07-physics-animation/03-physical-animation.md
// =============================================================================
#pragma once
#include "PhysicsEngine/PhysicalAnimationComponent.h"

class FPhysicalAnimHelper
{
public:
    /**
     * 配置上半身物理动画（受击晃动效果）
     * 需要在 BeginPlay 后调用（确保 Mesh 已初始化）
     */
    static void SetupUpperBodyPhysAnim(
        UPhysicalAnimationComponent* PhysAnim,
        USkeletalMeshComponent*      Mesh,
        FName                        PhysicsProfileName = FName("HitReaction"))
    {
        if (!PhysAnim || !Mesh) return;
        PhysAnim->SetSkeletalMeshComponent(Mesh);
        // 应用预设 Profile（在 PhAT 中定义）
        PhysAnim->ApplyPhysicsProfile(Mesh, PhysicsProfileName);
        // 开启 spine_01 以上骨骼物理
        Mesh->SetAllBodiesBelowSimulatePhysics(FName("spine_01"), true, true);
        // 初始权重：完全跟随动画
        Mesh->SetAllBodiesBelowPhysicsBlendWeight(FName("spine_01"), 1.f, false, true);
    }

    /** 平滑设置物理混合权重（0=纯物理，1=跟随动画目标）*/
    static void SetPhysicsBlendWeight(
        USkeletalMeshComponent* Mesh,
        FName                   TopBone,
        float                   Weight)
    {
        if (Mesh)
            Mesh->SetAllBodiesBelowPhysicsBlendWeight(
                TopBone, FMath::Clamp(Weight, 0.f, 1.f), false, true);
    }

    /** 受击时临时降低约束强度 */
    static void TriggerHitReaction(
        USkeletalMeshComponent* Mesh,
        FName HitBone, float ImpulseStrength,
        FVector ImpulseDirection, FVector HitLocation)
    {
        if (!Mesh) return;
        // 降低物理权重（让骨骼自由响应）
        SetPhysicsBlendWeight(Mesh, HitBone, 0.1f);
        // 施加冲量
        Mesh->AddImpulseAtLocation(
            ImpulseDirection.GetSafeNormal() * ImpulseStrength,
            HitLocation);
    }
};
