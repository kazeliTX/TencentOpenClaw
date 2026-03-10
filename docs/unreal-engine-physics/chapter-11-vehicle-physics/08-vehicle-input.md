# 11.8 输入处理与驾驶手感

> **难度**：⭐⭐⭐⭐☆

## 一、输入平滑（防止抖动）

```cpp
// 对转向输入做插值平滑，防止方向盘抖动
void AMyVehicle::SetupPlayerInputComponent(
    UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis("Throttle", this, &AMyVehicle::OnThrottle);
    PlayerInputComponent->BindAxis("Steering", this, &AMyVehicle::OnSteering);
    PlayerInputComponent->BindAction("Handbrake", IE_Pressed,
        this, &AMyVehicle::OnHandbrakePressed);
}

float TargetSteering = 0.f;
float CurrentSteering = 0.f;

void OnSteering(float Value)
{
    TargetSteering = Value;
}

void Tick(float Dt)
{
    Super::Tick(Dt);
    // 转向插值（模拟方向盘惯性）
    float InterpSpeed = FMath::Abs(TargetSteering) > FMath::Abs(CurrentSteering)
        ? 8.f   // 打方向快
        : 5.f;  // 回正慢
    CurrentSteering = FMath::FInterpTo(
        CurrentSteering, TargetSteering, Dt, InterpSpeed);
    GetVehicleMovementComponent()->SetSteeringInput(CurrentSteering);
    
    // 高速时减小转向量（防止高速甩尾）
    float Speed = GetVelocity().Size() / 100.f; // cm/s → m/s
    float SpeedFactor = FMath::Clamp(1.f - Speed / 100.f, 0.3f, 1.0f);
    GetVehicleMovementComponent()->SetSteeringInput(
        CurrentSteering * SpeedFactor);
}
```

## 二、转速表与仪表盘

```cpp
// 获取当前转速（用于 UI 仪表盘）
float GetRPM()
{
    return GetChaosVehicleMovement()->GetEngineRotationSpeed();
}

// 当前档位
int32 GetCurrentGear()
{
    return GetChaosVehicleMovement()->GetCurrentGear();
}

// 车速（km/h）
float GetSpeedKmh()
{
    return GetChaosVehicleMovement()->GetForwardSpeed() * 0.036f;
}
```
