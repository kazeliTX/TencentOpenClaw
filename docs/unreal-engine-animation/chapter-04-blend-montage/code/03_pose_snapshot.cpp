// =============================================================================
// 03_pose_snapshot.cpp
// Pose Snapshot 捕获与应用（布娃娃恢复过渡）
// 对应文档：chapter-04-blend-montage/07-pose-snapshots.md
// =============================================================================
#pragma once
#include "Animation/AnimInstance.h"
#include "Animation/PoseSnapshot.h"

class FPoseSnapshotHelper
{
public:
    /** 捕获当前骨骼姿势快照 */
    static bool CaptureSnapshot(
        USkeletalMeshComponent* Mesh,
        FPoseSnapshot& OutSnapshot)
    {
        if (!Mesh) return false;
        UAnimInstance* Anim = Mesh->GetAnimInstance();
        if (!Anim) return false;

        Anim->SnapshotPose(OutSnapshot);
        return true;
    }

    /** 捕获并存入 AnimInstance 变量（用于 AnimGraph 中的 Snapshot Pose 节点）*/
    static void CaptureToAnimInstance(
        USkeletalMeshComponent* Mesh,
        FName SnapshotVariableName)
    {
        // AnimInstance 需要有名为 SnapshotVariableName 的 FPoseSnapshot 属性
        // 通过反射系统赋值
        UAnimInstance* Anim = Mesh ? Mesh->GetAnimInstance() : nullptr;
        if (!Anim) return;

        FPoseSnapshot Snapshot;
        Anim->SnapshotPose(Snapshot);

        // 通过反射写入变量
        FProperty* Prop = Anim->GetClass()->FindPropertyByName(SnapshotVariableName);
        if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
        {
            void* ValuePtr = StructProp->ContainerPtrToValuePtr<void>(Anim);
            StructProp->CopyCompleteValue(ValuePtr, &Snapshot);
        }
    }
};

// =============================================================================
// 使用示例（在角色恢复站立时调用）：
//
// void AMyCharacter::OnRagdollEnd()
// {
//     // 1. 先捕获快照（在关闭物理之前！）
//     FPoseSnapshotHelper::CaptureToAnimInstance(GetMesh(), FName("RagdollSnapshot"));
//
//     // 2. 然后关闭物理（见第二章 Ragdoll 代码）
//     GetMesh()->SetSimulatePhysics(false);
//
//     // 3. AnimBP 中：Snapshot Pose 节点 → 读取 RagdollSnapshot
//     //    Blend 节点：Snapshot → StandUp Anim（Alpha 从 1→0 随时间过渡）
// }
// =============================================================================
