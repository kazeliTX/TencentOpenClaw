// =============================================================================
// 01_vehicle_setup.cpp
// Chaos Vehicle 完整配置：轮胎/悬挂/发动机/变速箱/输入
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "ChaosWheeledVehiclePawn.h"
#include "ChaosVehicleMovementComponent.h"
#include "MyVehicle.generated.h"

// ── 前轮 ──
UCLASS()
class UFrontWheel : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    UFrontWheel() {
        WheelRadius = 36.f; WheelWidth = 20.f;
        FrictionForceMultiplier = 2.5f; SlipThreshold = 20.f;
        SuspensionMaxRaise = 10.f; SuspensionMaxDrop = 14.f;
        SuspensionDampingRatio = 0.5f; SpringRate = 250.f;
        bAffectedBySteering = true;  MaxSteerAngle = 40.f;
        bAffectedByEngine   = false; bAffectedByHandbrake = false;
    }
};

// ── 后轮 ──
UCLASS()
class URearWheel : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    URearWheel() {
        WheelRadius = 38.f; WheelWidth = 24.f;
        FrictionForceMultiplier = 2.2f; SlipThreshold = 25.f;
        SuspensionMaxRaise = 8.f; SuspensionMaxDrop = 12.f;
        SuspensionDampingRatio = 0.45f; SpringRate = 300.f;
        bAffectedBySteering  = false;
        bAffectedByEngine    = true;   // 后驱
        bAffectedByHandbrake = true;   // 漂移
    }
};

// ── 车辆 Pawn ──
UCLASS()
class AMyVehicle : public AChaosWheeledVehiclePawn
{
    GENERATED_BODY()
public:
    float TargetSteering  = 0.f;
    float CurrentSteering = 0.f;

    AMyVehicle()
    {
        auto* Move = GetChaosVehicleMovement();

        // 轮子绑定
        Move->WheelSetups.Add({ UFrontWheel::StaticClass(), FName("wheel_FL") });
        Move->WheelSetups.Add({ UFrontWheel::StaticClass(), FName("wheel_FR") });
        Move->WheelSetups.Add({ URearWheel::StaticClass(),  FName("wheel_RL") });
        Move->WheelSetups.Add({ URearWheel::StaticClass(),  FName("wheel_RR") });

        // 发动机
        FChaosVehicleEngineData E;
        E.MaxRPM = 7500.f; E.MaxTorque = 350.f;
        E.EngineIdleRPM = 900.f; E.EngineRevUpRate = 800.f; E.EngineRevDownRate = 600.f;
        Move->EngineSetup = E;

        // 变速箱
        FChaosVehicleTransmissionData T;
        T.bUseAutomaticGears = true;
        T.ForwardGearRatios = {2.8f, 1.9f, 1.4f, 1.0f, 0.8f, 0.65f};
        T.ReverseGearRatios = {2.5f};
        T.FinalRatio = 3.5f;
        T.ChangeUpRPM = 6500.f; T.ChangeDownRPM = 3000.f;
        T.GearChangeTime = 0.4f;
        Move->TransmissionSetup = T;

        // 差速器（AWD 平衡）
        Move->DifferentialSetup.DifferentialType = EVehicleDifferential::AllWheelDrive;
        Move->DifferentialSetup.FrontRearSplit = 0.45f; // 前 45% 后 55%
    }

    virtual void SetupPlayerInputComponent(UInputComponent* IC) override
    {
        Super::SetupPlayerInputComponent(IC);
        IC->BindAxis("Throttle",  this, &AMyVehicle::OnThrottle);
        IC->BindAxis("Steering",  this, &AMyVehicle::OnSteering);
        IC->BindAxis("Brake",     this, &AMyVehicle::OnBrake);
        IC->BindAction("Handbrake", IE_Pressed,
            this, &AMyVehicle::OnHandbrakeOn);
        IC->BindAction("Handbrake", IE_Released,
            this, &AMyVehicle::OnHandbrakeOff);
    }

    void OnThrottle(float V) { GetChaosVehicleMovement()->SetThrottleInput(V); }
    void OnBrake(float V)    { GetChaosVehicleMovement()->SetBrakeInput(V);    }
    void OnHandbrakeOn()     { GetChaosVehicleMovement()->SetHandbrakeInput(true);  }
    void OnHandbrakeOff()    { GetChaosVehicleMovement()->SetHandbrakeInput(false); }
    void OnSteering(float V) { TargetSteering = V; }

    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);

        // 转向插值
        float InterpSpeed = FMath::Abs(TargetSteering) > FMath::Abs(CurrentSteering) ? 8.f : 5.f;
        CurrentSteering = FMath::FInterpTo(CurrentSteering, TargetSteering, Dt, InterpSpeed);

        // 高速减小转向量
        float SpeedMps = GetVelocity().Size() * 0.01f;
        float SpeedFactor = FMath::Clamp(1.f - SpeedMps / 80.f, 0.25f, 1.0f);
        GetChaosVehicleMovement()->SetSteeringInput(CurrentSteering * SpeedFactor);
    }
};
