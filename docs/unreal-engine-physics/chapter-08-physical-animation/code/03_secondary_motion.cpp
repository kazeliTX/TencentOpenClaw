// =============================================================================
// 03_secondary_motion.cpp
// 二级运动：AnimDynamics 配置辅助 / 物理骨骼附件
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "SecondaryMotionSetup.generated.h"

UCLASS()
class ASecondaryMotionCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UPhysicalAnimationComponent* PhysAnim;

    // 饰品/头发专用 PhysAnim 设置
    void SetupAttachmentPhysics()
    {
        // 马尾辫骨骼（ponytail_01, ponytail_02, ponytail_03）
        // 极弱追随 → 大量物理自由度
        FPhysicalAnimationData HairData;
        HairData.bIsLocalSimulation      = true;
        HairData.OrientationStrength     = 15.f;   // 非常弱
        HairData.AngularVelocityStrength = 3.f;    // 低阻尼，摆动明显
        HairData.MaxAngularForce         = 5000.f;

        for (FName Bone : {
            FName("ponytail_01"), FName("ponytail_02"),
            FName("ponytail_03"), FName("ponytail_04") })
        {
            PhysAnim->ApplyPhysicalAnimationSettings(Bone, HairData);
            GetMesh()->SetBodySimulatePhysics(Bone, true);
        }

        // 耳环（ear_jewel_l/r）：非常轻，快速摆动
        FPhysicalAnimationData EarringData;
        EarringData.bIsLocalSimulation      = true;
        EarringData.OrientationStrength     = 8.f;
        EarringData.AngularVelocityStrength = 1.5f;
        EarringData.MaxAngularForce         = 2000.f;

        for (FName Bone : {
            FName("ear_jewel_l"), FName("ear_jewel_r") })
        {
            PhysAnim->ApplyPhysicalAnimationSettings(Bone, EarringData);
            GetMesh()->SetBodySimulatePhysics(Bone, true);
        }

        // 腰带饰件：中等摆动
        FPhysicalAnimationData BeltData;
        BeltData.bIsLocalSimulation      = true;
        BeltData.OrientationStrength     = 40.f;
        BeltData.AngularVelocityStrength = 8.f;
        BeltData.MaxAngularForce         = 10000.f;

        PhysAnim->ApplyPhysicalAnimationSettings(FName("belt_prop"), BeltData);
        GetMesh()->SetBodySimulatePhysics(FName("belt_prop"), true);
    }
};
