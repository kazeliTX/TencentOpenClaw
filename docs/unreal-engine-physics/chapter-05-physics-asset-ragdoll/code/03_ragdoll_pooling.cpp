// =============================================================================
// 03_ragdoll_pooling.cpp
// Ragdoll 对象池：复用已死亡的骨骼网格，控制同时活跃数量
// 对应文档：chapter-05/09-performance-optimization.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "RagdollPool.generated.h"

// 单个 Ragdoll 实例
USTRUCT()
struct FRagdollInstance
{
    GENERATED_BODY()
    UPROPERTY() USkeletalMeshComponent* Mesh = nullptr;
    bool  bActive    = false;
    float DeathTime  = 0.f;    // 死亡时刻（GetWorld()->TimeSeconds）
    float Distance   = 0.f;   // 上次到玩家的距离缓存
};

UCLASS()
class ARagdollPool : public AActor
{
    GENERATED_BODY()
public:
    // 最大同时活跃 Ragdoll 数
    static const int32 MaxActive = 16;
    // 超过此距离停止物理
    static const float FreezeDistance;
    // 尸体存在时长
    static const float CorpseLifespan;

    TArray<FRagdollInstance> Pool;

    // ─────────────────────────────────────────────
    // 请求激活一个 Ragdoll（返回 Mesh，供外部初始化）
    // ─────────────────────────────────────────────
    USkeletalMeshComponent* AcquireRagdoll(USkeletalMesh* SKMesh)
    {
        // 清理过期的
        CleanupExpired();

        // 超过上限 → 回收最远/最老的
        TArray<FRagdollInstance*> Active;
        for (FRagdollInstance& R : Pool)
            if (R.bActive) Active.Add(&R);

        if (Active.Num() >= MaxActive)
        {
            FRagdollInstance* Oldest = nullptr;
            for (FRagdollInstance* R : Active)
            {
                if (!Oldest ||
                    R->DeathTime < Oldest->DeathTime ||
                    R->Distance  > Oldest->Distance + 500.f)
                    Oldest = R;
            }
            if (Oldest) RecycleRagdoll(*Oldest);
        }

        // 找空闲槽或新建
        FRagdollInstance* Slot = nullptr;
        for (FRagdollInstance& R : Pool)
        {
            if (!R.bActive) { Slot = &R; break; }
        }
        if (!Slot)
        {
            Pool.AddDefaulted();
            Slot = &Pool.Last();
            Slot->Mesh = NewObject<USkeletalMeshComponent>(this);
            Slot->Mesh->RegisterComponent();
        }

        // 初始化 Mesh
        Slot->Mesh->SetSkeletalMesh(SKMesh);
        Slot->Mesh->SetVisibility(true);
        Slot->Mesh->SetSimulatePhysics(true);
        Slot->bActive   = true;
        Slot->DeathTime = GetWorld()->GetTimeSeconds();
        return Slot->Mesh;
    }

    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);

        APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
        if (!Player) return;

        for (FRagdollInstance& R : Pool)
        {
            if (!R.bActive || !R.Mesh) continue;
            R.Distance = FVector::Distance(
                R.Mesh->GetComponentLocation(),
                Player->GetActorLocation());

            // 距离过远 → 冻结物理
            if (R.Distance > FreezeDistance && R.Mesh->IsSimulatingPhysics())
                R.Mesh->SetSimulatePhysics(false);
            else if (R.Distance <= FreezeDistance && !R.Mesh->IsSimulatingPhysics())
                R.Mesh->SetSimulatePhysics(true);
        }
    }

private:
    void CleanupExpired()
    {
        float Now = GetWorld()->GetTimeSeconds();
        for (FRagdollInstance& R : Pool)
        {
            if (R.bActive && Now - R.DeathTime > CorpseLifespan)
                RecycleRagdoll(R);
        }
    }

    void RecycleRagdoll(FRagdollInstance& R)
    {
        if (R.Mesh) R.Mesh->SetVisibility(false);
        if (R.Mesh) R.Mesh->SetSimulatePhysics(false);
        R.bActive = false;
    }
};

const float ARagdollPool::FreezeDistance = 2000.f;
const float ARagdollPool::CorpseLifespan = 15.f;
