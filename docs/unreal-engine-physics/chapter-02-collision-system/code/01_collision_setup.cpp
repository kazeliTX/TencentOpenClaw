// =============================================================================
// 01_collision_setup.cpp
// 运行时碰撞通道/响应/Profile 动态设置与切换
// 对应文档：chapter-02-collision-system/02~04
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "CollisionSetupDemo.generated.h"

UCLASS()
class MYPROJECT_API ACollisionSetupDemo : public ACharacter
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* WeaponMesh;
    UPROPERTY(VisibleAnywhere) USphereComponent*     TriggerRange;
    bool bIsInvincible = false;

    ACollisionSetupDemo()
    {
        // 武器 Mesh
        WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
        WeaponMesh->SetupAttachment(GetMesh(), FName("hand_r"));
        WeaponMesh->SetCollisionProfileName(FName("Weapon"));  // 自定义武器 Profile

        // 技能触发范围
        TriggerRange = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerRange"));
        TriggerRange->SetupAttachment(RootComponent);
        TriggerRange->SetSphereRadius(300.f);
        TriggerRange->SetCollisionProfileName(FName("OverlapAllDynamic"));
    }

    // ─────────────────────────────────────────────
    // 完整碰撞配置示例：玩家角色死亡变 Ragdoll
    // ─────────────────────────────────────────────
    void SetupRagdollCollision()
    {
        USkeletalMeshComponent* SkelMesh = GetMesh();
        
        // 1. 胶囊（主碰撞体）禁用碰撞
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        // 2. 骨骼网格切换为 Ragdoll Profile
        SkelMesh->SetCollisionProfileName(FName("Ragdoll"));
        
        // 3. 启用所有骨骼物理模拟
        SkelMesh->SetSimulatePhysics(true);
        SkelMesh->SetAllBodiesSimulatePhysics(true);
        
        // 4. 武器禁用碰撞（避免干扰布娃娃）
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        
        // 5. 技能范围也禁用
        TriggerRange->SetGenerateOverlapEvents(false);
    }

    // ─────────────────────────────────────────────
    // 无敌状态：对 WeaponTrace 通道 Ignore
    // ─────────────────────────────────────────────
    void SetInvincible(bool bInvincible)
    {
        bIsInvincible = bInvincible;
        ECollisionResponse Response = bInvincible ? ECR_Ignore : ECR_Block;
        
        // 对武器追踪通道调整响应
        GetMesh()->SetCollisionResponseToChannel(
            ECC_GameTraceChannel1,  // WeaponTrace
            Response);
        
        UE_LOG(LogTemp, Log, TEXT("Invincible: %d"), bInvincible);
    }

    // ─────────────────────────────────────────────
    // 运行时动态切换 Profile（如切换状态机）
    // ─────────────────────────────────────────────
    void SetState_Ghost()
    {
        // 幽灵状态：穿墙，但保留与 Pawn 的 Overlap
        FCollisionResponseContainer RC;
        RC.SetAllChannels(ECR_Ignore);
        RC.SetResponse(ECC_Pawn, ECR_Overlap);
        RC.SetResponse(ECC_WorldStatic, ECR_Ignore);  // 穿墙！
        GetMesh()->SetCollisionResponseToChannels(RC);
    }

    void SetState_Normal()
    {
        // 恢复正常
        GetMesh()->SetCollisionProfileName(FName("CharacterMesh"));
    }

    // ─────────────────────────────────────────────
    // 团队碰撞管理
    // ─────────────────────────────────────────────
    void SetTeamIgnore(ACharacter* Teammate, bool bIgnore)
    {
        // 移动时互相忽略（不阻挡队友行走）
        GetCapsuleComponent()->IgnoreActorWhenMoving(Teammate, bIgnore);
        Teammate->GetCapsuleComponent()->IgnoreActorWhenMoving(this, bIgnore);
        
        // Trace 检测时也忽略（友军不互相造成伤害）
        if (bIgnore)
        {
            GetMesh()->SetCollisionResponseToChannel(
                ECC_GameTraceChannel1,  // WeaponTrace
                ECR_Ignore);
        }
    }

    // ─────────────────────────────────────────────
    // 打印当前完整碰撞配置（调试）
    // ─────────────────────────────────────────────
    void DebugPrintCollisionInfo(UPrimitiveComponent* Comp) const
    {
        if (!Comp) return;
        UE_LOG(LogTemp, Log, TEXT("=== Collision: %s ==="), *Comp->GetName());
        UE_LOG(LogTemp, Log, TEXT("  Profile: %s"),
            *Comp->GetCollisionProfileName().ToString());
        UE_LOG(LogTemp, Log, TEXT("  Enabled: %d"),
            (int)Comp->GetCollisionEnabled());
        UE_LOG(LogTemp, Log, TEXT("  ObjectType: %d"),
            (int)Comp->GetCollisionObjectType());
        
        const FCollisionResponseContainer& RC = Comp->GetCollisionResponseToChannels();
        static const char* ChannelNames[] = {
            "WorldStatic","WorldDynamic","Pawn","Visibility","Camera",
            "PhysicsBody","Vehicle","Destructible"
        };
        for (int32 i = 0; i < 8; ++i)
        {
            ECollisionResponse R = RC.GetResponse((ECollisionChannel)i);
            UE_LOG(LogTemp, Log, TEXT("  [%s]: %d"), 
                ANSI_TO_TCHAR(ChannelNames[i]), (int)R);
        }
    }
};
