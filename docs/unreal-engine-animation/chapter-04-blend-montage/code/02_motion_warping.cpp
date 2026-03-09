// =============================================================================
// 02_motion_warping.cpp
// Motion Warping 目标管理（翻越/攀爬/交互动作对齐）
// 对应文档：chapter-04-blend-montage/09-motion-warping.md
// =============================================================================
#pragma once
#include "MotionWarpingComponent.h"

class FMotionWarpingHelper
{
public:
    /** 设置单目标 Warp（翻越障碍物）*/
    static void SetVaultTarget(
        UMotionWarpingComponent* MWC,
        FVector TargetLocation,
        FRotator TargetFacing)
    {
        if (!MWC) return;
        MWC->AddOrUpdateWarpTargetFromLocationAndRotation(
            FName("VaultTarget"),
            TargetLocation,
            TargetFacing.Quaternion()
        );
    }

    /** 设置双目标 Warp（攀爬：手 + 脚）*/
    static void SetClimbTargets(
        UMotionWarpingComponent* MWC,
        FVector HandLocation,
        FVector FootLocation,
        FRotator Facing)
    {
        if (!MWC) return;
        MWC->AddOrUpdateWarpTargetFromLocationAndRotation(
            FName("ClimbHand"), HandLocation, Facing.Quaternion());
        MWC->AddOrUpdateWarpTargetFromLocationAndRotation(
            FName("ClimbFoot"), FootLocation, Facing.Quaternion());
    }

    /** 从 HitResult 设置 Warp 目标（常见用法）*/
    static void SetWarpFromHit(
        UMotionWarpingComponent* MWC,
        const FHitResult& Hit,
        FName TargetName,
        float HeightOffset = 0.0f)
    {
        if (!MWC || !Hit.bBlockingHit) return;
        FVector Loc = Hit.ImpactPoint + FVector(0, 0, HeightOffset);
        FRotator Rot = Hit.ImpactNormal.Rotation();
        MWC->AddOrUpdateWarpTargetFromLocationAndRotation(TargetName, Loc, Rot.Quaternion());
    }

    /** 清理所有 Warp 目标 */
    static void ClearAllTargets(UMotionWarpingComponent* MWC, TArray<FName> TargetNames)
    {
        if (!MWC) return;
        for (const FName& Name : TargetNames)
            MWC->RemoveWarpTarget(Name);
    }
};
