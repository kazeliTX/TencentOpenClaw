// =============================================================================
// 03_substepping_demo.cpp
// 子步进回调注册，在子步进中安全施加力，固定时间步进积分
// 对应文档：chapter-01-physics-fundamentals/08-substepping.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "SubsteppingDemoActor.generated.h"

UCLASS()
class MYPROJECT_API ASubsteppingDemoActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
    
    // ── 在游戏线程存储，子步进回调中读取 ──
    // （子步进运行在物理线程，不能直接访问游戏线程数据）
    UPROPERTY(EditAnywhere, Category="Substep")
    float ThrustForce = 50000.f;      // 推力大小（N = kg*cm/s²）
    UPROPERTY(EditAnywhere, Category="Substep")
    float AirResistanceCoeff = 0.001f; // 空气阻力系数
    UPROPERTY(EditAnywhere, Category="Substep")
    bool  bEnableThrust = true;

    // 存储子步进注册句柄（用于注销）
    FDelegateHandle SubstepHandle;
    
    // 游戏线程安全缓存（每帧在Tick中更新，子步进只读）
    FVector CachedForwardVector;
    float   CachedMass = 1.f;

    ASubsteppingDemoActor()
    {
        PrimaryActorTick.bCanEverTick = true;
        Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
        RootComponent = Mesh;
    }

    // ─────────────────────────────────────────────
    // BeginPlay：注册子步进回调
    // ─────────────────────────────────────────────
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        
        Mesh->SetSimulatePhysics(true);
        
        // 缓存初始值
        CachedForwardVector = GetActorForwardVector();
        if (FBodyInstance* BI = Mesh->GetBodyInstance())
            CachedMass = BI->GetBodyMass();
        
        // 注册子步进回调
        if (FPhysScene* PhysScene = GetWorld()->GetPhysicsScene())
        {
            // 回调签名：void(float DeltaTime, FBodyInstance* BodyInst)
            SubstepHandle = PhysScene->OnPhysicsSubstep.AddUObject(
                this, &ASubsteppingDemoActor::OnPhysicsSubstep);
            UE_LOG(LogTemp, Log, TEXT("Substep callback registered"));
        }
    }

    // ─────────────────────────────────────────────
    // EndPlay：注销子步进回调（防止野指针）
    // ─────────────────────────────────────────────
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override
    {
        Super::EndPlay(EndPlayReason);
        
        if (SubstepHandle.IsValid())
        {
            if (FPhysScene* PhysScene = GetWorld()->GetPhysicsScene())
                PhysScene->OnPhysicsSubstep.Remove(SubstepHandle);
            SubstepHandle.Reset();
            UE_LOG(LogTemp, Log, TEXT("Substep callback unregistered"));
        }
    }

    // ─────────────────────────────────────────────
    // Tick（游戏线程）：更新缓存数据，供子步进读取
    // ─────────────────────────────────────────────
    virtual void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);
        
        // 在游戏线程安全地更新缓存（子步进只读这些值）
        CachedForwardVector = GetActorForwardVector();
        if (FBodyInstance* BI = Mesh->GetBodyInstance())
            CachedMass = BI->GetBodyMass();
    }

    // ─────────────────────────────────────────────
    // 子步进回调（物理线程！）
    //   DeltaTime = 子步长（固定值，如 0.016666s）
    //   BodyInst  = 当前刚体的 FBodyInstance
    // 注意：不能在这里调用游戏线程 API！
    //        只能操作 FBodyInstance 和读取缓存数据
    // ─────────────────────────────────────────────
    void OnPhysicsSubstep(float DeltaTime, FBodyInstance* BodyInst)
    {
        if (!BodyInst) return;
        
        // ── 1. 推进力（方向依赖缓存的 ForwardVector）──
        if (bEnableThrust)
        {
            FVector Thrust = CachedForwardVector * ThrustForce;
            BodyInst->AddForce(Thrust, false, false);
            // 参数说明：
            //   第二个 bool = bAccelChange（false=力，true=加速度）
            //   第三个 bool = bIsLocalForce（false=世界坐标，true=局部）
        }
        
        // ── 2. 空气阻力（平方项，更真实）──
        FVector Vel = BodyInst->GetUnrealWorldVelocity();
        float Speed = Vel.Size();
        if (Speed > 1.f)
        {
            float DragMag = AirResistanceCoeff * Speed * Speed * CachedMass;
            FVector DragForce = -Vel.GetSafeNormal() * DragMag;
            BodyInst->AddForce(DragForce, false, false);
        }
        
        // ── 3. 速度限制（子步进中限制速度，更精确）──
        const float MaxSpeed = 1000.f; // cm/s
        if (Speed > MaxSpeed)
        {
            FVector ClampedVel = Vel.GetSafeNormal() * MaxSpeed;
            BodyInst->SetLinearVelocity(ClampedVel, false);
        }
        
        // ── 4. 旋转稳定（防止翻滚）──
        FVector AngVel = BodyInst->GetUnrealWorldAngularVelocityInRadians();
        if (AngVel.Size() > 2.f) // 超过 2 rad/s 才处理
        {
            // 施加反向角冲量（不是力矩！冲量更稳定）
            FVector StabilizeTorque = -AngVel * CachedMass * 0.1f;
            BodyInst->AddTorqueInRadians(StabilizeTorque, false, false);
        }
    }

    // ─────────────────────────────────────────────
    // 演示：固定时间步进手动积分（不依赖 UE 物理，完全自定义）
    // 用途：极简物理对象（不需要碰撞检测，只需要运动轨迹）
    // ─────────────────────────────────────────────
    struct FSimpleRigidBody
    {
        FVector Position;
        FVector Velocity;
        float   Mass;

        void StepForward(float Dt, FVector ExternalForce)
        {
            // 显式欧拉积分（简单但有能量漂移）
            FVector Accel = ExternalForce / Mass;
            Velocity += Accel * Dt;
            Position += Velocity * Dt;
            
            // Verlet 积分（能量更守恒）
            // Position_new = 2*Position - Position_prev + Accel * Dt²
            // （需要记录 Position_prev）
        }
    };
    
    FSimpleRigidBody TestBody;
    float FixedStepAccum = 0.f;
    const float FixedStep = 0.016666f;

    UFUNCTION(BlueprintCallable, Category="Substep")
    void StepFixedPhysics(float GameDeltaTime)
    {
        FixedStepAccum += GameDeltaTime;
        int Steps = 0;
        
        while (FixedStepAccum >= FixedStep && Steps < 6)
        {
            FVector Gravity(0, 0, -980.f);  // cm/s²
            TestBody.StepForward(FixedStep, Gravity * TestBody.Mass);
            FixedStepAccum -= FixedStep;
            Steps++;
        }
        
        UE_LOG(LogTemp, Verbose, TEXT("Fixed steps this frame: %d, pos: %s"),
            Steps, *TestBody.Position.ToString());
    }
};
