// =============================================================================
// 01_control_rig_cpp.cpp
// C++ 运行时调用 Control Rig / 设置控制器值
// 对应文档：chapter-10-control-rig/06-ik-nodes.md
// =============================================================================
#pragma once
#include "ControlRig.h"
#include "Units/RigUnitContext.h"

// 运行时通过 AnimBP 的 Control Rig 节点传递 IK 目标
// 推荐：通过 AnimInstance 变量（UPROPERTY）传递，Control Rig 读取该变量
// 直接 C++ API 操作 Control Rig 示例：

void AMyCharacter::UpdateControlRigIKTargets(
    FVector LeftHandTarget,
    FVector RightHandTarget)
{
    USkeletalMeshComponent* Mesh = GetMesh();
    if (!Mesh) return;

    // 获取 AnimInstance（需要是 UControlRigAnimInstance 或使用 Control Rig 节点的 AnimInstance）
    UAnimInstance* AnimInst = Mesh->GetAnimInstance();
    if (!AnimInst) return;

    // 推荐方式：在 AnimInstance 上暴露 UPROPERTY，Control Rig 读取
    // 直接在 AnimBP 的 NativeUpdateAnimation 中：
    //   LeftHandIKTarget  = LeftHandTarget;
    //   RightHandIKTarget = RightHandTarget;
    // Control Rig 通过 Get Variable 节点读取这些值

    // 如果直接操作 UControlRig：
    if (UControlRig* CR = FindComponentByClass<UControlRig>())
    {
        // 设置控制器位置
        CR->SetControlGlobalTransform(
            FName("ctrl_hand_l"),
            FTransform(LeftHandTarget));
        CR->SetControlGlobalTransform(
            FName("ctrl_hand_r"),
            FTransform(RightHandTarget));
    }
}
