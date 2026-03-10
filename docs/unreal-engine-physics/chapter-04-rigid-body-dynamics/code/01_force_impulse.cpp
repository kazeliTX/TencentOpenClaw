// =============================================================================
// 01_force_impulse.cpp
// 力/冲量/力矩/角冲量完整 API 示例 + SubstepTick 中安全施力
// 对应文档：chapter-04/01-forces-overview.md ~ 04-explosion-force.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "ForceImpulseDemo.generated.h"

UCLASS()
class AForceImpulseDemo : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
    UPROPERTY(VisibleAnywhere) URadialForceComponent* RadialForce;
    FDelegateHandle SubstepHandle;

    // 缓存（供SubstepTick读取，由游戏线程写入）
    FVector CachedThrustDir = FVector::ForwardVector;
    float   ThrustMagnitude = 0.f;
    bool    bThrustEnabled  = false;

    AForceImpulseDemo()
    {
        PrimaryActorTick.bCanEverTick = true;
        Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
        Mesh->SetSimulatePhysics(true);
        RootComponent = Mesh;
        RadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("Radial"));
        RadialForce->SetupAttachment(Mesh);
        RadialForce->Radius = 500.f;
        RadialForce->ImpulseStrength = 100000.f;
        RadialForce->Falloff = RIF_Linear;
        RadialForce->bIgnoreOwningActor = true;
    }

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        SubstepHandle = GetWorld()->GetPhysicsScene()->OnPhysicsSubstep
            .AddUObject(this, &AForceImpulseDemo::SubstepTick);
    }

    virtual void EndPlay(const EEndPlayReason::Type R) override
    {
        Super::EndPlay(R);
        if (SubstepHandle.IsValid())
            GetWorld()->GetPhysicsScene()->OnPhysicsSubstep.Remove(SubstepHandle);
    }

    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);
        CachedThrustDir = GetActorForwardVector();
    }

    // ── 子步进回调（物理线程）──
    void SubstepTick(float SubDt, FBodyInstance* BI)
    {
        if (!BI || !bThrustEnabled) return;

        // 推进力（帧率无关）
        BI->AddForce(CachedThrustDir * ThrustMagnitude, false, false);

        // 空气阻力
        FVector V = BI->GetUnrealWorldVelocity();
        float S = V.Size();
        if (S > 1.f)
            BI->AddForce(-V.GetSafeNormal() * S * S * 0.002f * BI->GetBodyMass(),
                false, false);

        // 速度钳制
        if (S > 2000.f)
            BI->SetLinearVelocity(V.GetSafeNormal() * 2000.f, false);
    }

    // ────────────────────── 力 API ──────────────────────
    UFUNCTION(BlueprintCallable) void Demo_AddForce()
    {
        // 普通力（质量影响加速度）
        Mesh->AddForce(FVector(0, 0, 100000.f)); // 向上 100kN

        // 加速度模式（质量无关）
        Mesh->AddForce(FVector(0, 0, 500.f), NAME_None, true); // +500 cm/s² 向上

        // 局部力（跟随物体朝向）
        Mesh->AddLocalForce(FVector(50000.f, 0, 0)); // 沿本地 X 轴（前进方向）

        // 在指定世界坐标点施加力（产生力矩）
        FVector ApplyPoint = GetActorLocation() + FVector(50, 0, 80);
        Mesh->AddForceAtLocation(FVector(0, 50000, 0), ApplyPoint);
    }

    // ────────────────────── 冲量 API ──────────────────────
    UFUNCTION(BlueprintCallable) void Demo_AddImpulse()
    {
        // 普通冲量（Δv = J/m）
        Mesh->AddImpulse(FVector(0, 0, 30000.f)); // 向上瞬间冲量

        // 速度变化模式（直接设 Δv，质量无关）
        Mesh->AddImpulse(FVector(0, 0, 400.f), NAME_None, true); // 直接加 400cm/s

        // 在指定点施加冲量（线+角冲量）
        FVector HitPoint = GetActorLocation() + FVector(0, 60, 0);
        Mesh->AddImpulseAtLocation(FVector(10000, 0, 5000), HitPoint);
    }

    // ────────────────────── 力矩/角冲量 API ──────────────────────
    UFUNCTION(BlueprintCallable) void Demo_Torque()
    {
        // 力矩（持续旋转）
        Mesh->AddTorqueInDegrees(FVector(0, 0, 50000.f)); // 绕 Z 旋转

        // 角冲量（瞬间旋转）
        Mesh->AddAngularImpulseInDegrees(FVector(0, 0, 360.f)); // 瞬间转 360°
    }

    // ────────────────────── 速度控制 API ──────────────────────
    UFUNCTION(BlueprintCallable) void Demo_Velocity()
    {
        Mesh->SetPhysicsLinearVelocity(FVector(500, 0, 200));   // 直接设速度
        Mesh->SetPhysicsAngularVelocityInDegrees(FVector(0, 0, 90)); // 直接设角速度

        // 读取
        FVector LinVel = Mesh->GetPhysicsLinearVelocity();
        FVector AngVel = Mesh->GetPhysicsAngularVelocityInDegrees();
        UE_LOG(LogTemp, Log, TEXT("Vel: %s  AngVel: %s"),
            *LinVel.ToString(), *AngVel.ToString());
    }

    // ────────────────────── 一次性爆炸冲量 ──────────────────────
    UFUNCTION(BlueprintCallable) void TriggerExplosion()
    {
        RadialForce->FireImpulse();
        UE_LOG(LogTemp, Log, TEXT("Explosion at %s, R=%.0f, I=%.0f"),
            *GetActorLocation().ToString(),
            RadialForce->Radius, RadialForce->ImpulseStrength);
    }
};
