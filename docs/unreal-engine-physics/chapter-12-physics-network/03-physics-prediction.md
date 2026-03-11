# 12.3 物理预测与回滚

> **难度**：⭐⭐⭐⭐⭐

## 一、预测框架（Network Physics Prediction）

```cpp
// UE5 物理预测接口（INetworkPhysicsInterface）
// 需要实现 ProduceInput / InjectInput / PreSimulate

// 车辆/角色的物理输入结构
USTRUCT()
struct FPhysicsInputs
{
    GENERATED_BODY()
    float Throttle  = 0.f;
    float Steering  = 0.f;
    bool  bHandbrake = false;
};

// 实现预测组件
UCLASS()
class UMyPhysicsPredictionComponent
    : public UActorComponent
    , public INetworkPhysicsInterface
{
    GENERATED_BODY()
public:
    // 客户端产生本地输入（每帧）
    virtual void ProduceInput(
        const int32 DeltaSeconds, FBaseInput* Input) override
    {
        FPhysicsInputs* PI = static_cast<FPhysicsInputs*>(Input);
        PI->Throttle   = PlayerController->GetThrottle();
        PI->Steering   = PlayerController->GetSteering();
        PI->bHandbrake = PlayerController->IsHandbrakeDown();
    }

    // 服务端/客户端应用输入到物理
    virtual void SimulatePrePhysics(
        const int32 Dt, FBaseInput* Input) override
    {
        FPhysicsInputs* PI = static_cast<FPhysicsInputs*>(Input);
        VehicleComp->SetThrottleInput(PI->Throttle);
        VehicleComp->SetSteeringInput(PI->Steering);
        VehicleComp->SetHandbrakeInput(PI->bHandbrake);
    }
};
```

## 二、回滚阈值配置

```
p.net.PhysicsResimulation.PositionErrorThreshold 5.0   ← 位置误差超过 5cm 才触发回滚
p.net.PhysicsResimulation.VelocityErrorThreshold 50.0  ← 速度误差超过 50cm/s
p.net.PhysicsResimulation.AngularVelocityErrorThreshold 5.0

这些阈值控制客户端何时触发 Resimulation：
  太小 → 频繁回滚（抖动）
  太大 → 误差累积（位置不一致）
  建议：根据游戏类型调整：
    FPS（毫米级精度）：5cm / 50cm/s
    赛车（厘米级精度）：20cm / 200cm/s
    策略游戏（宽松）：50cm / 500cm/s
```
