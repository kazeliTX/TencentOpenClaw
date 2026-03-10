# 10.4 流体与角色交互

> **难度**：⭐⭐⭐⭐⭐

## 一、进入/离开水体检测

```cpp
// 基于水面高度检测角色是否在水中
void AMyCharacter::Tick(float Dt)
{
    Super::Tick(Dt);
    
    float WaterZ = GetWaterHeight(GetWorld(), GetActorLocation());
    float FeetZ  = GetActorLocation().Z;
    
    bool bInWater = (FeetZ < WaterZ);
    if (bInWater != bWasInWater)
    {
        bWasInWater = bInWater;
        if (bInWater)   OnEnterWater(WaterZ);
        else            OnExitWater();
    }
    
    // 浮力计算（简化版）
    if (bInWater)
    {
        float SubmergeDepth = WaterZ - FeetZ;
        float BuoyancyForce = SubmergeDepth * 50.f; // N
        GetMesh()->AddForce(FVector(0,0,BuoyancyForce));
    }
}

void OnEnterWater(float WaterSurfaceZ)
{
    // 切换游泳动画状态
    GetCharacterMovement()->SetMovementMode(MOVE_Swimming);
    // 产生入水溅水效果
    UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(), SplashVFX,
        FVector(GetActorLocation().X,
                GetActorLocation().Y, WaterSurfaceZ));
}

bool bWasInWater = false;
```

## 二、脚步水花效果

```
角色走过浅水时的涟漪：
  1. 脚部 IK 检测接触点高度
  2. 如果接触点高度 ≈ 水面高度（±5cm）
  3. 生成 Niagara Fluid 外力（产生涟漪）
  4. 同时生成水花粒子特效

与 Niagara Fluid 联动：
  脚步事件 → SplashAtLocation(FootPos, 0.3f)
  → Niagara Grid 在脚部位置产生向外扩散的波纹
```
