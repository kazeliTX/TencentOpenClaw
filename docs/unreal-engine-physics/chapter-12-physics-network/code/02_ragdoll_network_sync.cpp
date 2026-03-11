// =============================================================================
// 02_ragdoll_network_sync.cpp
// 布娃娃网络同步：Multicast 触发 + 本地模拟策略
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RagdollNetworkSync.generated.h"

UCLASS()
class ANetworkRagdollCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    // ─── 服务端调用：触发布娃娃 ───
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void AuthorityDie(FVector DeathImpulse, FName HitBone)
    {
        if (!HasAuthority()) return;
        // 广播给所有客户端（含服务端自己）
        Multicast_TriggerRagdoll(DeathImpulse, HitBone);
    }

    // ─── 所有端执行布娃娃激活 ───
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_TriggerRagdoll(FVector DeathImpulse, FName HitBone)
    {
        GetCharacterMovement()->DisableMovement();
        GetCapsuleComponent()->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
        GetMesh()->SetAllBodiesSimulatePhysics(true);
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

        if (HitBone != NAME_None)
            GetMesh()->AddImpulseAtLocation(
                DeathImpulse,
                GetMesh()->GetBoneLocation(HitBone),
                HitBone);
        
        // 布娃娃后不再需要高频复制（各端本地模拟）
        SetReplicateMovement(false);
        NetUpdateFrequency = 0.f;
    }

    // ─── 命中反应（Physical Animation 同步）───
    UFUNCTION(Server, Reliable)
    void Server_HitReaction(FVector ImpactPoint, FVector ImpactDir,
        float ImpulseMag, FName BoneName)
    {
        // 服务端验证 → 广播给所有人
        Multicast_HitReaction(ImpactPoint, ImpactDir, ImpulseMag, BoneName);
    }

    UFUNCTION(NetMulticast, Unreliable) // 命中反应不需要可靠（偶尔丢包无所谓）
    void Multicast_HitReaction(FVector ImpactPoint, FVector ImpactDir,
        float ImpulseMag, FName BoneName)
    {
        GetMesh()->AddImpulseAtLocation(
            ImpactDir * ImpulseMag, ImpactPoint, BoneName);
        // PhysicalAnimation 在各端本地响应
    }
};
