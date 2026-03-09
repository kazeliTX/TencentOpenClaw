// =============================================================================
// 03_custom_notify.cpp
// 自定义 AnimNotify 合集（脚步、攻击判定、特效）
// 对应文档：chapter-03-anim-blueprint/09-notify-system.md
// =============================================================================
#pragma once
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CustomAnimNotifies.generated.h"

// ─────────────────────────────────────────────────────────
// 1. 脚步音效 Notify（点通知）
// ─────────────────────────────────────────────────────────
UCLASS(meta=(DisplayName="Footstep"))
class UAnimNotify_Footstep : public UAnimNotify
{
    GENERATED_BODY()

    virtual void Notify(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventRef) override
    {
        if (!MeshComp) return;

        // 脚骨骼世界位置
        FVector FootLoc = MeshComp->GetBoneLocation(FootBoneName);

        // 向下 Line Trace 检测地面材质
        FHitResult Hit;
        FVector Start = FootLoc + FVector(0,0,5);
        FVector End   = FootLoc - FVector(0,0,50);
        bool bHit = MeshComp->GetWorld()->LineTraceSingleByChannel(
            Hit, Start, End, ECC_WorldStatic);

        // 根据材质播放音效
        if (bHit && Hit.PhysMaterial.IsValid())
        {
            // 实际项目：查表 PhysicalMaterial → Sound
            // 这里简化为直接播放
            if (DefaultFootstepSound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    MeshComp, DefaultFootstepSound, FootLoc,
                    FRotator::ZeroRotator, Volume);
            }
        }
    }

public:
    UPROPERTY(EditAnywhere) FName FootBoneName = FName("foot_l");
    UPROPERTY(EditAnywhere) TObjectPtr<USoundBase> DefaultFootstepSound;
    UPROPERTY(EditAnywhere, meta=(ClampMin="0", ClampMax="1")) float Volume = 0.8f;
};

// ─────────────────────────────────────────────────────────
// 2. 攻击判定开始（点通知）
// ─────────────────────────────────────────────────────────
UCLASS(meta=(DisplayName="Begin Attack Trace"))
class UAnimNotify_BeginAttackTrace : public UAnimNotify
{
    GENERATED_BODY()

    virtual void Notify(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventRef) override
    {
        if (!MeshComp) return;

        // 通过接口通知角色开始攻击判定
        if (AActor* Owner = MeshComp->GetOwner())
        {
            // 使用 UInterface 避免强耦合（推荐）
            // IAttackInterface::Execute_OnBeginAttackTrace(Owner, AttackSocketName);
            Owner->Tags.Add(FName("AttackActive")); // 简化示例
        }
    }

public:
    UPROPERTY(EditAnywhere) FName AttackSocketName = FName("weapon_tip");
};

// ─────────────────────────────────────────────────────────
// 3. 攻击判定窗口（区间通知）
// ─────────────────────────────────────────────────────────
UCLASS(meta=(DisplayName="Attack Window"))
class UAnimNotifyState_AttackWindow : public UAnimNotifyState
{
    GENERATED_BODY()

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation, float TotalDuration,
        const FAnimNotifyEventReference& EventRef) override
    {
        if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
            Owner->Tags.Add(FName("InAttackWindow"));
    }

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventRef) override
    {
        if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
            Owner->Tags.Remove(FName("InAttackWindow"));
    }

    virtual void NotifyTick(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation, float FrameDeltaTime,
        const FAnimNotifyEventReference& EventRef) override
    {
        // 每帧扫描碰撞（如需精确的挥刀轨迹检测）
        // 实际项目中通常在 Component 中做，不在 Notify Tick 中
    }
};

// ─────────────────────────────────────────────────────────
// 4. 特效 Spawn（点通知，带变换控制）
// ─────────────────────────────────────────────────────────
UCLASS(meta=(DisplayName="Spawn VFX at Bone"))
class UAnimNotify_SpawnVFX : public UAnimNotify
{
    GENERATED_BODY()

    virtual void Notify(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventRef) override
    {
        if (!MeshComp || !VFXTemplate) return;

        FTransform SocketTransform = MeshComp->GetSocketTransform(
            SocketName, RTS_World);

        // 生成 Niagara 特效
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            MeshComp->GetWorld(),
            VFXTemplate,
            SocketTransform.GetLocation(),
            SocketTransform.GetRotation().Rotator(),
            FVector(1.f), // Scale
            bAutoDestroy
        );
    }

public:
    UPROPERTY(EditAnywhere) FName SocketName = FName("weapon_muzzle");
    UPROPERTY(EditAnywhere) TObjectPtr<class UNiagaraSystem> VFXTemplate;
    UPROPERTY(EditAnywhere) bool bAutoDestroy = true;
};
