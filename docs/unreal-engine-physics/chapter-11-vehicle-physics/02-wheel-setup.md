# 11.2 轮胎配置

> **难度**：⭐⭐⭐⭐☆

## 一、UChaosVehicleWheel 参数

```cpp
// 创建轮胎类（继承 UChaosVehicleWheel）
UCLASS()
class UMyCarWheel_Front : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    UMyCarWheel_Front()
    {
        // 几何
        WheelRadius   = 35.f;   // cm，轮胎半径
        WheelWidth    = 20.f;   // cm，轮胎宽度
        
        // 摩擦
        FrictionForceMultiplier    = 2.0f;  // 摩擦力倍率（越大抓地力越强）
        SideSlipModifier           = 1.0f;  // 侧向打滑修正
        SlipThreshold              = 20.f;  // 开始打滑的速度阈值（km/h）
        SkidThreshold              = 40.f;  // 开始烧胎的速度阈值
        
        // 悬挂
        SuspensionMaxRaise  = 10.f;   // 最大上行程（cm）
        SuspensionMaxDrop   = 15.f;   // 最大下行程（cm）
        SuspensionDampingRatio = 0.5f; // 悬挂阻尼比（0.5=临界阻尼）
        
        // 转向（前轮 = true）
        bAffectedBySteering = true;
        MaxSteerAngle       = 40.f;   // 最大转向角（度）
        
        // 驱动（后驱车后轮 = true）
        bAffectedByEngine = false;    // 前轮不驱动（RWD）
        bAffectedByHandbrake = false; // 前轮不受手刹
    }
};

// 后轮
UCLASS()
class UMyCarWheel_Rear : public UChaosVehicleWheel
{
    GENERATED_BODY()
public:
    UMyCarWheel_Rear()
    {
        WheelRadius = 38.f;   // 后轮略大（跑车常见）
        WheelWidth  = 25.f;
        
        FrictionForceMultiplier = 2.2f; // 后轮摩擦略大（驱动轮）
        
        bAffectedBySteering  = false;  // 后轮不转向
        bAffectedByEngine    = true;   // 后驱
        bAffectedByHandbrake = true;   // 后轮手刹（漂移）
        
        SuspensionMaxRaise = 8.f;
        SuspensionMaxDrop  = 12.f;
    }
};
```

## 二、WheelSetup 配置

```cpp
// 在 VehicleMovementComponent 中配置每个轮子
Movement->WheelSetups.Add({
    .WheelClass   = UMyCarWheel_Front::StaticClass(),
    .BoneName     = FName("wheel_FL"),
    .AdditionalOffset = FVector(0, 0, 0)  // 相对骨骼的额外偏移
});
```
