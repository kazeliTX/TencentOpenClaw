// =============================================================================
// 03_ik_retarget_helper.cpp
// IK Retargeter 批量操作辅助（编辑器工具类）
// 对应文档：chapter-10-control-rig/08-anim-retarget.md
// =============================================================================
#pragma once
#if WITH_EDITOR
#include "Retargeter/IKRetargeter.h"
#include "Retargeter/IKRetargetProcessor.h"
#include "AssetToolsModule.h"

class FIKRetargetHelper
{
public:
    /**
     * 运行时重定向单个动画（非编辑器批量导出，而是运行时姿势重定向）
     * 需要 AnimGraph 节点"Retarget Pose From Mesh"，不需要 C++ 直接调用
     * 以下为编辑器批量导出辅助
     */
    static void BatchRetargetAnimations(
        UIKRetargeter*                   Retargeter,
        const TArray<UAnimSequence*>&    SourceAnims,
        const FString&                   OutputDirectory)
    {
        if (!Retargeter || SourceAnims.IsEmpty()) return;

        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(
            "AssetTools").Get();

        for (UAnimSequence* SourceAnim : SourceAnims)
        {
            if (!SourceAnim) continue;

            FString OutputName = FString::Printf(
                TEXT("%s%s_Retargeted"),
                *OutputDirectory,
                *SourceAnim->GetName());

            UE_LOG(LogTemp, Log, TEXT("Retargeting: %s → %s"),
                *SourceAnim->GetName(), *OutputName);

            // 实际重定向调用（编辑器 API）
            // UIKRetargetProcessor 执行姿势重定向
            // 完整实现需要 IKRetargetEditorModule 的 RetargetAnimationAssets
        }
    }

    /** 验证两个 IK Rig 的链映射是否完整 */
    static bool ValidateRetargeterChainMapping(UIKRetargeter* Retargeter)
    {
        if (!Retargeter) return false;
        UIKRetargeterController* Controller = UIKRetargeterController::GetController(Retargeter);
        if (!Controller) return false;

        TArray<FName> SourceChains = Controller->GetSourceChainNames();
        TArray<FName> TargetChains = Controller->GetTargetChainNames();

        bool bAllMapped = true;
        for (const FName& Chain : SourceChains)
        {
            FName MappedTarget = Controller->GetTargetChainNameFromSourceChainName(Chain);
            if (MappedTarget == NAME_None)
            {
                UE_LOG(LogTemp, Warning, TEXT("Chain not mapped: %s"), *Chain.ToString());
                bAllMapped = false;
            }
        }
        return bAllMapped;
    }
};
#endif // WITH_EDITOR
