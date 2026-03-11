// =============================================================================
// 01_physics_replication.cpp
// 物理复制：刚体同步/动态更新频率/睡眠优化
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsReplicationActor.generated.h"

UCLASS()
class APhysicsReplicationActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;

    APhysicsReplicationActor()
    {
        bReplicates = true;
        SetReplicateMovement(true);
        NetUpdateFrequency    = 30.f;
        MinNetUpdateFrequency = 5.f;

        Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
        Mesh->SetSimulatePhysics(true);
        RootComponent = Mesh;
    }

    // ─── 服务端 Tick：动态调整复制频率 ───
    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);
        if (!HasAuthority()) return;

        bool bSleeping = Mesh->RigidBodyIsAsleep();
        float Speed    = Mesh->GetPhysicsLinearVelocity().Size();

        if (bSleeping)
            NetUpdateFrequency = 1.f;          // 静止：极低频
        else if (Speed > 500.f)
            NetUpdateFrequency = 60.f;         // 高速：高频
        else
            NetUpdateFrequency = 20.f;         // 慢速：中频
    }

    // ─── 服务端施加冲量（客户端请求 → 服务端执行）───
    UFUNCTION(Server, Reliable)
    void Server_ApplyImpulse(FVector Impulse, FVector WorldPoint)
    {
        Mesh->AddImpulseAtLocation(Impulse, WorldPoint);
    }

    // ─── 客户端调用入口（自动转发到服务端）───
    UFUNCTION(BlueprintCallable)
    void ApplyImpulse(FVector Impulse, FVector WorldPoint)
    {
        if (HasAuthority())
            Mesh->AddImpulseAtLocation(Impulse, WorldPoint);
        else
            Server_ApplyImpulse(Impulse, WorldPoint);
    }
};
