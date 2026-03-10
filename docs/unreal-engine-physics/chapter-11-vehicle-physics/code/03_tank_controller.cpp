// =============================================================================
// 03_tank_controller.cpp
// 坦克/履带车：差速转向 + 履带动画驱动
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "TankController.generated.h"

UCLASS()
class ATankPawn : public AChaosWheeledVehiclePawn
{
    GENERATED_BODY()
public:
    // 坦克炮塔旋转（独立于车身）
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* TurretMesh;
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* BarrelMesh;

    float TurretYaw   = 0.f;
    float BarrelPitch = 0.f;

    virtual void SetupPlayerInputComponent(UInputComponent* IC) override
    {
        // 差速转向：左摇杆 Y 轴 = 前进，X 轴 = 转向
        IC->BindAxis("MoveForward", this, &ATankPawn::OnMoveForward);
        IC->BindAxis("MoveRight",   this, &ATankPawn::OnTurn);
        // 炮塔
        IC->BindAxis("TurretYaw",   this, &ATankPawn::OnTurretYaw);
        IC->BindAxis("BarrelPitch", this, &ATankPawn::OnBarrelPitch);
    }

    // 差速转向：左右履带速度差 → 转向
    void OnMoveForward(float V)
    {
        GetChaosVehicleMovement()->SetThrottleInput(V > 0 ? V : 0.f);
        GetChaosVehicleMovement()->SetBrakeInput(V < 0 ? -V : 0.f);
    }

    void OnTurn(float V)
    {
        GetChaosVehicleMovement()->SetSteeringInput(V);
        // 坦克原地转向：一侧油门正，一侧刹车
        // 在 CustomVehicleMovement 中实现完整差速
    }

    void OnTurretYaw(float V)
    {
        TurretYaw += V * 60.f * GetWorld()->GetDeltaSeconds();
        TurretYaw  = FMath::Fmod(TurretYaw, 360.f);
        TurretMesh->SetRelativeRotation(FRotator(0, TurretYaw, 0));
    }

    void OnBarrelPitch(float V)
    {
        BarrelPitch = FMath::Clamp(BarrelPitch + V * 30.f *
            GetWorld()->GetDeltaSeconds(), -5.f, 25.f); // 仰角 -5~25°
        BarrelMesh->SetRelativeRotation(FRotator(BarrelPitch, 0, 0));
    }
};
