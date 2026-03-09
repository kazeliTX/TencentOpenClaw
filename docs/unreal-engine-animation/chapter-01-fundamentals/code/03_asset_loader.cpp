// =============================================================================
// 03_asset_loader.cpp
// 动画资产运行时加载示例
// 演示：异步加载 AnimSequence、动态切换 AnimBP、运行时播放 Montage
// 对应文档：chapter-01-fundamentals/04-animation-asset-types.md
// =============================================================================

#pragma once
#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Engine/StreamableManager.h"
#include "AnimAssetLoader.generated.h"

// ─────────────────────────────────────────────────────────────
// 动画资产加载工具组件
// 挂载到角色上使用
// ─────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Animation), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UAnimAssetLoader : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimAssetLoader()
    {
        PrimaryComponentTick.bCanEverTick = false;
    }

    // ─────────────────────────────────────────────────────────
    // 1. 同步加载（适合小资产，会造成帧卡顿）
    // ─────────────────────────────────────────────────────────

    /**
     * 同步加载并立即播放 AnimMontage
     * @param MontageAssetPath 资产路径（如 "/Game/Anims/AM_Attack"）
     * @param PlayRate 播放速率
     * @return 播放时长（秒），失败返回 0
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    float LoadAndPlayMontage(const FString& MontageAssetPath, float PlayRate = 1.0f)
    {
        // 同步加载资产（阻塞式，小心使用）
        UAnimMontage* Montage = LoadObject<UAnimMontage>(
            nullptr,
            *MontageAssetPath
        );

        if (!Montage)
        {
            UE_LOG(LogTemp, Error,
                TEXT("LoadAndPlayMontage: Failed to load '%s'"),
                *MontageAssetPath);
            return 0.0f;
        }

        // 获取 Owner 的 AnimInstance
        APawn* Owner = Cast<APawn>(GetOwner());
        if (!Owner) return 0.0f;

        ACharacter* Character = Cast<ACharacter>(Owner);
        if (!Character) return 0.0f;

        UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();
        if (!AnimInst) return 0.0f;

        return AnimInst->Montage_Play(Montage, PlayRate);
    }

    // ─────────────────────────────────────────────────────────
    // 2. 异步加载（推荐，不阻塞主线程）
    // ─────────────────────────────────────────────────────────

    /**
     * 异步加载 AnimMontage，加载完成后自动播放
     * @param MontageSoftRef 软引用（在编辑器中配置，不占内存直到需要时加载）
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void AsyncLoadAndPlayMontage(
        TSoftObjectPtr<UAnimMontage> MontageSoftRef,
        float PlayRate = 1.0f)
    {
        if (MontageSoftRef.IsNull())
        {
            UE_LOG(LogTemp, Warning, TEXT("AsyncLoadAndPlayMontage: Null reference!"));
            return;
        }

        // 如果已经加载，直接播放
        if (MontageSoftRef.IsValid())
        {
            PlayMontageInternal(MontageSoftRef.Get(), PlayRate);
            return;
        }

        // 异步加载（不阻塞主线程）
        FStreamableManager& StreamableManager =
            UAssetManager::GetStreamableManager();

        StreamableManager.RequestAsyncLoad(
            MontageSoftRef.ToSoftObjectPath(),
            FStreamableDelegate::CreateUObject(
                this,
                &UAnimAssetLoader::OnMontageLoaded,
                MontageSoftRef,
                PlayRate
            )
        );

        UE_LOG(LogTemp, Log,
            TEXT("AsyncLoadAndPlayMontage: Loading '%s'..."),
            *MontageSoftRef.ToString());
    }

    /**
     * 异步加载 AnimBP 并动态切换
     * 适用场景：不同武器对应不同 AnimBP，武器切换时动态替换
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void AsyncSwitchAnimBlueprint(
        TSoftClassPtr<UAnimInstance> AnimBPSoftClass)
    {
        if (AnimBPSoftClass.IsNull()) return;

        if (AnimBPSoftClass.IsValid())
        {
            ApplyAnimBlueprintClass(AnimBPSoftClass.Get());
            return;
        }

        FStreamableManager& StreamableManager =
            UAssetManager::GetStreamableManager();

        StreamableManager.RequestAsyncLoad(
            AnimBPSoftClass.ToSoftObjectPath(),
            FStreamableDelegate::CreateUObject(
                this,
                &UAnimAssetLoader::OnAnimBPLoaded,
                AnimBPSoftClass
            )
        );
    }

    // ─────────────────────────────────────────────────────────
    // 3. 批量预加载（关卡加载时调用）
    // ─────────────────────────────────────────────────────────

    /**
     * 批量预加载一组动画资产到内存
     * 建议在关卡开始时调用，确保战斗中不会出现加载卡顿
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PreloadAnimationAssets(
        const TArray<TSoftObjectPtr<UAnimMontage>>& MontageList)
    {
        TArray<FSoftObjectPath> PathsToLoad;
        for (const TSoftObjectPtr<UAnimMontage>& Ref : MontageList)
        {
            if (!Ref.IsNull() && !Ref.IsValid())
            {
                PathsToLoad.Add(Ref.ToSoftObjectPath());
            }
        }

        if (PathsToLoad.Num() == 0) return;

        FStreamableManager& StreamableManager =
            UAssetManager::GetStreamableManager();

        // 批量加载（共享一个加载句柄，更高效）
        PreloadHandle = StreamableManager.RequestAsyncLoad(
            PathsToLoad,
            FStreamableDelegate::CreateUObject(
                this,
                &UAnimAssetLoader::OnPreloadComplete
            ),
            FStreamableManager::AsyncLoadHighPriority
        );

        UE_LOG(LogTemp, Log,
            TEXT("PreloadAnimationAssets: Loading %d montages..."),
            PathsToLoad.Num());
    }

    /** 释放预加载的资产（关卡结束时调用，释放内存）*/
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ReleasePreloadedAssets()
    {
        if (PreloadHandle.IsValid())
        {
            PreloadHandle->ReleaseHandle();
            PreloadHandle.Reset();
        }
    }

private:
    // 预加载句柄（持有引用，防止 GC）
    TSharedPtr<FStreamableHandle> PreloadHandle;

    void PlayMontageInternal(UAnimMontage* Montage, float PlayRate)
    {
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        if (!Character) return;

        UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();
        if (!AnimInst) return;

        float Duration = AnimInst->Montage_Play(Montage, PlayRate);
        UE_LOG(LogTemp, Log,
            TEXT("PlayMontage: '%s' (%.2fs)"),
            *Montage->GetName(), Duration);
    }

    void ApplyAnimBlueprintClass(TSubclassOf<UAnimInstance> AnimBPClass)
    {
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        if (!Character) return;

        Character->GetMesh()->SetAnimInstanceClass(AnimBPClass);
        UE_LOG(LogTemp, Log,
            TEXT("SwitchAnimBP: Switched to '%s'"),
            *AnimBPClass->GetName());
    }

    // 异步加载完成回调
    void OnMontageLoaded(
        TSoftObjectPtr<UAnimMontage> MontageSoftRef,
        float PlayRate)
    {
        UAnimMontage* Montage = MontageSoftRef.Get();
        if (Montage)
        {
            PlayMontageInternal(Montage, PlayRate);
        }
    }

    void OnAnimBPLoaded(TSoftClassPtr<UAnimInstance> AnimBPSoftClass)
    {
        if (AnimBPSoftClass.IsValid())
        {
            ApplyAnimBlueprintClass(AnimBPSoftClass.Get());
        }
    }

    void OnPreloadComplete()
    {
        UE_LOG(LogTemp, Log, TEXT("PreloadAnimationAssets: All assets loaded!"));
    }
};

// =============================================================================
// 使用示例（在 Character 的 .h 中）：
//
// // 软引用（在编辑器中配置，运行时按需加载）
// UPROPERTY(EditAnywhere, Category = "Animation")
// TSoftObjectPtr<UAnimMontage> AttackMontage;
//
// UPROPERTY(EditAnywhere, Category = "Animation")
// TSoftClassPtr<UAnimInstance> RifleAnimBPClass;
//
// // 在 BeginPlay 中预加载
// void AMyCharacter::BeginPlay()
// {
//     Super::BeginPlay();
//     TArray<TSoftObjectPtr<UAnimMontage>> ToPreload = { AttackMontage, ... };
//     AnimLoader->PreloadAnimationAssets(ToPreload);
// }
//
// // 攻击时播放
// void AMyCharacter::Attack()
// {
//     AnimLoader->AsyncLoadAndPlayMontage(AttackMontage);
// }
// =============================================================================
