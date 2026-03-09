// =============================================================================
// 02_skeleton_query.cpp
// 骨骼查询工具函数集合
// 演示：骨骼变换查询、Socket 操作、骨骼层级遍历
// 对应文档：chapter-01-fundamentals/02-skeleton-asset.md
// =============================================================================

#pragma once
#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/Skeleton.h"

// ─────────────────────────────────────────────────────────────
// 骨骼查询工具类（静态方法集合）
// ─────────────────────────────────────────────────────────────

struct FSkeletonQueryUtils
{
    // ─────────────────────────────────────────────────────────
    // 骨骼变换查询
    // ─────────────────────────────────────────────────────────

    /** 获取骨骼的世界变换（通过骨骼名称）*/
    static FTransform GetBoneWorldTransform(
        USkeletalMeshComponent* Mesh,
        FName BoneName)
    {
        if (!Mesh) return FTransform::Identity;

        int32 BoneIdx = Mesh->GetBoneIndex(BoneName);
        if (BoneIdx == INDEX_NONE)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("GetBoneWorldTransform: Bone '%s' not found!"),
                *BoneName.ToString());
            return FTransform::Identity;
        }

        // GetBoneTransform 默认返回 Component Space，再乘以组件世界变换
        FTransform CompSpaceTrans = Mesh->GetBoneTransform(BoneIdx);
        return CompSpaceTrans * Mesh->GetComponentTransform();
    }

    /** 获取骨骼的 Component Space 变换 */
    static FTransform GetBoneComponentTransform(
        USkeletalMeshComponent* Mesh,
        FName BoneName)
    {
        if (!Mesh) return FTransform::Identity;

        int32 BoneIdx = Mesh->GetBoneIndex(BoneName);
        if (BoneIdx == INDEX_NONE) return FTransform::Identity;

        return Mesh->GetBoneTransform(BoneIdx);  // 默认 Component Space
    }

    /** 获取两根骨骼之间的相对变换 */
    static FTransform GetRelativeTransformBetweenBones(
        USkeletalMeshComponent* Mesh,
        FName FromBone,
        FName ToBone)
    {
        FTransform FromWorld = GetBoneWorldTransform(Mesh, FromBone);
        FTransform ToWorld   = GetBoneWorldTransform(Mesh, ToBone);
        return ToWorld.GetRelativeTransform(FromWorld);
    }

    // ─────────────────────────────────────────────────────────
    // Socket 操作
    // ─────────────────────────────────────────────────────────

    /** 安全获取 Socket 世界位置（Socket 不存在时返回 FVector::ZeroVector）*/
    static FVector GetSocketWorldLocation(
        USkeletalMeshComponent* Mesh,
        FName SocketName)
    {
        if (!Mesh) return FVector::ZeroVector;
        if (!Mesh->DoesSocketExist(SocketName))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("GetSocketWorldLocation: Socket '%s' not found!"),
                *SocketName.ToString());
            return FVector::ZeroVector;
        }
        return Mesh->GetSocketLocation(SocketName);
    }

    /** 获取 Socket 完整世界变换 */
    static FTransform GetSocketWorldTransform(
        USkeletalMeshComponent* Mesh,
        FName SocketName)
    {
        if (!Mesh) return FTransform::Identity;
        return Mesh->GetSocketTransform(SocketName, RTS_World);
    }

    /** 将 Actor Attach 到指定 Socket */
    static bool AttachActorToSocket(
        AActor* Actor,
        USkeletalMeshComponent* Mesh,
        FName SocketName,
        bool bSnapToSocket = true)
    {
        if (!Actor || !Mesh) return false;
        if (!Mesh->DoesSocketExist(SocketName)) return false;

        EAttachmentRule Rule = bSnapToSocket
            ? EAttachmentRule::SnapToTarget
            : EAttachmentRule::KeepRelative;

        Actor->AttachToComponent(
            Mesh,
            FAttachmentTransformRules(Rule, Rule, EAttachmentRule::KeepWorld, false),
            SocketName
        );
        return true;
    }

    // ─────────────────────────────────────────────────────────
    // 骨骼层级遍历
    // ─────────────────────────────────────────────────────────

    /** 获取某骨骼的所有子骨骼名称（递归） */
    static void GetChildBoneNames(
        USkeletalMeshComponent* Mesh,
        FName ParentBoneName,
        TArray<FName>& OutChildren,
        bool bRecursive = true)
    {
        if (!Mesh || !Mesh->GetSkeletalMeshAsset()) return;

        const FReferenceSkeleton& RefSkel =
            Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();

        int32 ParentIdx = RefSkel.FindBoneIndex(ParentBoneName);
        if (ParentIdx == INDEX_NONE) return;

        int32 NumBones = RefSkel.GetNum();
        for (int32 i = 0; i < NumBones; ++i)
        {
            int32 ThisParent = RefSkel.GetParentIndex(i);
            if (ThisParent == ParentIdx)
            {
                FName ChildName = RefSkel.GetBoneName(i);
                OutChildren.Add(ChildName);

                if (bRecursive)
                {
                    GetChildBoneNames(Mesh, ChildName, OutChildren, true);
                }
            }
        }
    }

    /** 获取骨骼的父骨骼名称 */
    static FName GetParentBoneName(
        USkeletalMeshComponent* Mesh,
        FName BoneName)
    {
        if (!Mesh || !Mesh->GetSkeletalMeshAsset()) return NAME_None;

        const FReferenceSkeleton& RefSkel =
            Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();

        int32 BoneIdx   = RefSkel.FindBoneIndex(BoneName);
        int32 ParentIdx = RefSkel.GetParentIndex(BoneIdx);

        if (ParentIdx == INDEX_NONE) return NAME_None;  // 根骨骼无父级
        return RefSkel.GetBoneName(ParentIdx);
    }

    /** 打印骨骼层级到输出日志（调试用）*/
    static void DebugPrintBoneHierarchy(
        USkeletalMeshComponent* Mesh,
        FName StartBone = NAME_None,
        int32 Indent = 0)
    {
        if (!Mesh || !Mesh->GetSkeletalMeshAsset()) return;

        const FReferenceSkeleton& RefSkel =
            Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();

        // 如果没有指定起始骨骼，从根骨骼开始
        int32 StartIdx = 0;
        if (StartBone != NAME_None)
        {
            StartIdx = RefSkel.FindBoneIndex(StartBone);
            if (StartIdx == INDEX_NONE) return;
        }

        // 递归打印
        FString IndentStr = FString::ChrN(Indent * 2, ' ');
        FName BoneName = RefSkel.GetBoneName(StartIdx);
        UE_LOG(LogTemp, Log, TEXT("%s[%d] %s"), *IndentStr, StartIdx, *BoneName.ToString());

        int32 NumBones = RefSkel.GetNum();
        for (int32 i = 0; i < NumBones; ++i)
        {
            if (RefSkel.GetParentIndex(i) == StartIdx)
            {
                DebugPrintBoneHierarchy(Mesh, RefSkel.GetBoneName(i), Indent + 1);
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // 坐标空间转换工具
    // ─────────────────────────────────────────────────────────

    /** 将世界空间坐标转换为 Component Space */
    static FVector WorldToComponentSpace(
        USkeletalMeshComponent* Mesh,
        FVector WorldLocation)
    {
        if (!Mesh) return WorldLocation;
        return Mesh->GetComponentTransform().InverseTransformPosition(WorldLocation);
    }

    /** 将 Component Space 坐标转换为世界空间 */
    static FVector ComponentToWorldSpace(
        USkeletalMeshComponent* Mesh,
        FVector ComponentLocation)
    {
        if (!Mesh) return ComponentLocation;
        return Mesh->GetComponentTransform().TransformPosition(ComponentLocation);
    }

    /** 将世界空间方向向量转换为 Component Space（不受位移影响）*/
    static FVector WorldToComponentDirection(
        USkeletalMeshComponent* Mesh,
        FVector WorldDirection)
    {
        if (!Mesh) return WorldDirection;
        return Mesh->GetComponentTransform().InverseTransformVector(WorldDirection);
    }
};

// =============================================================================
// 使用示例：
//
// // 获取头部骨骼世界位置
// FTransform HeadWorld = FSkeletonQueryUtils::GetBoneWorldTransform(
//     GetMesh(), FName("head"));
//
// // 获取武器 Socket 位置（用于生成特效）
// FVector MuzzlePos = FSkeletonQueryUtils::GetSocketWorldLocation(
//     GetMesh(), FName("muzzle_flash"));
//
// // 将武器 Actor Attach 到手部 Socket
// FSkeletonQueryUtils::AttachActorToSocket(
//     WeaponActor, GetMesh(), FName("weapon_r"));
//
// // 调试：打印所有骨骼层级
// FSkeletonQueryUtils::DebugPrintBoneHierarchy(GetMesh());
//
// // 获取从 pelvis 开始的所有骨骼
// TArray<FName> LegBones;
// FSkeletonQueryUtils::GetChildBoneNames(
//     GetMesh(), FName("pelvis"), LegBones);
// =============================================================================
