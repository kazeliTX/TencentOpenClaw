// =============================================================================
// 01_enable_physics.cpp
// 运行时物理控制：启用/禁用模拟，设置质量/阻尼，Sleep/Wake 控制
// 对应文档：chapter-01-physics-fundamentals/04-physics-actor-setup.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsPublic.h"
#include "PhysicsControlActor.generated.h"

UCLASS()
class MYPROJECT_API APhysicsControlActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;

    APhysicsControlActor()
    {
        PrimaryActorTick.bCanEverTick = true;
        Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
        RootComponent = Mesh;
    }

    // ─────────────────────────────────────────────
    // 启用物理模拟
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Physics")
    void EnablePhysics(bool bSimulate = true)
    {
        // 设置 Mobility（必须是 Movable 才能物理模拟）
        Mesh->SetMobility(EComponentMobility::Movable);
        // 启用/禁用物理模拟
        Mesh->SetSimulatePhysics(bSimulate);
        // 启用重力
        Mesh->SetEnableGravity(bSimulate);
        
        UE_LOG(LogTemp, Log, TEXT("Physics %s for %s"),
            bSimulate ? TEXT("ENABLED") : TEXT("DISABLED"),
            *GetName());
    }

    // ─────────────────────────────────────────────
    // 设置质量（kg）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Physics")
    void SetMassKg(float MassKg)
    {
        FBodyInstance* BI = Mesh->GetBodyInstance();
        if (!BI) return;

        // Override Mass = true，手动设置质量
        BI->bOverrideMass = true;
        BI->SetMassOverride(MassKg);
        // 更新质量属性（惯性张量等跟着更新）
        BI->UpdateMassProperties();

        UE_LOG(LogTemp, Log, TEXT("Mass set to %.1f kg (actual: %.1f kg)"),
            MassKg, BI->GetBodyMass());
    }

    // ─────────────────────────────────────────────
    // 设置阻尼
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Physics")
    void SetDamping(float LinearDamping, float AngularDamping)
    {
        Mesh->SetLinearDamping(LinearDamping);
        Mesh->SetAngularDamping(AngularDamping);
        
        // 验证
        FBodyInstance* BI = Mesh->GetBodyInstance();
        if (BI)
        {
            UE_LOG(LogTemp, Log, TEXT("Damping: Lin=%.2f Ang=%.2f"),
                BI->LinearDamping, BI->AngularDamping);
        }
    }

    // ─────────────────────────────────────────────
    // 设置质心偏移（局部坐标，cm）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Physics")
    void SetCenterOfMassOffset(FVector Offset)
    {
        FBodyInstance* BI = Mesh->GetBodyInstance();
        if (!BI) return;
        
        BI->COMNudge = Offset;
        BI->UpdateMassProperties();
        
        // 验证质心世界位置
        FVector CoM = Mesh->GetBodyInstance()->GetCOMPosition();
        UE_LOG(LogTemp, Log, TEXT("CoM world pos: %s"), *CoM.ToString());
    }

    // ─────────────────────────────────────────────
    // 设置惯性张量缩放
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Physics")
    void SetInertiaTensorScale(FVector Scale)
    {
        FBodyInstance* BI = Mesh->GetBodyInstance();
        if (!BI) return;
        
        BI->InertiaTensorScale = Scale;
        BI->UpdateMassProperties();
        
        FVector IT = BI->GetBodyInertiaTensor();
        UE_LOG(LogTemp, Log, TEXT("Inertia Tensor: %s (kg*cm²)"), *IT.ToString());
    }

    // ─────────────────────────────────────────────
    // CCD（连续碰撞检测）开关
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Physics")
    void SetCCDEnabled(bool bEnabled)
    {
        FBodyInstance* BI = Mesh->GetBodyInstance();
        if (!BI) return;
        
        BI->bUseCCD = bEnabled;
        UE_LOG(LogTemp, Log, TEXT("CCD %s"), bEnabled ? TEXT("ON") : TEXT("OFF"));
    }

    // ─────────────────────────────────────────────
    // Sleep / Wake 控制
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Physics")
    void ForceSleep()
    {
        Mesh->PutRigidBodyToSleep();
        UE_LOG(LogTemp, Log, TEXT("Forced to sleep"));
    }

    UFUNCTION(BlueprintCallable, Category="Physics")
    void ForceWake()
    {
        Mesh->WakeRigidBody();
        UE_LOG(LogTemp, Log, TEXT("Woken up"));
    }

    UFUNCTION(BlueprintCallable, Category="Physics")
    bool IsAsleep() const
    {
        return Mesh->RigidBodyIsAsleep();
    }

    // ─────────────────────────────────────────────
    // 锁定 / 解锁旋转轴（如防止 Z 轴翻转）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Physics")
    void LockRotation(bool bLockX, bool bLockY, bool bLockZ)
    {
        FBodyInstance* BI = Mesh->GetBodyInstance();
        if (!BI) return;
        
        BI->bLockXRotation = bLockX;
        BI->bLockYRotation = bLockY;
        BI->bLockZRotation = bLockZ;
        // 必须调用此函数使锁定生效
        BI->CreateDOFLock();
        
        UE_LOG(LogTemp, Log, TEXT("Rotation locks: X=%d Y=%d Z=%d"),
            bLockX, bLockY, bLockZ);
    }

    // ─────────────────────────────────────────────
    // 输出当前物理状态（调试用）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Physics")
    void PrintPhysicsState() const
    {
        FBodyInstance* BI = Mesh->GetBodyInstance();
        if (!BI) return;
        
        UE_LOG(LogTemp, Log, TEXT("=== Physics State: %s ==="), *GetName());
        UE_LOG(LogTemp, Log, TEXT("  SimulatePhysics: %d"), Mesh->IsSimulatingPhysics());
        UE_LOG(LogTemp, Log, TEXT("  Mass: %.2f kg"), BI->GetBodyMass());
        UE_LOG(LogTemp, Log, TEXT("  LinearVelocity: %s cm/s"), *Mesh->GetPhysicsLinearVelocity().ToString());
        UE_LOG(LogTemp, Log, TEXT("  AngularVelocity: %s rad/s"), *Mesh->GetPhysicsAngularVelocityInRadians().ToString());
        UE_LOG(LogTemp, Log, TEXT("  IsAsleep: %d"), Mesh->RigidBodyIsAsleep());
        UE_LOG(LogTemp, Log, TEXT("  LinearDamping: %.3f"), BI->LinearDamping);
        UE_LOG(LogTemp, Log, TEXT("  AngularDamping: %.3f"), BI->AngularDamping);
        UE_LOG(LogTemp, Log, TEXT("  GravityScale: %.3f"), BI->GravityScale);
    }
};
