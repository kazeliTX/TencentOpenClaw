# 12.5 刚体同步实现

> **难度**：⭐⭐⭐⭐⭐

## 一、AActor 物理同步基础

```cpp
UCLASS()
class APhysicsActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;

    APhysicsActor()
    {
        // 开启 Actor 复制
        bReplicates = true;
        SetReplicateMovement(true);  // 自动同步位置/旋转/速度
        
        Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
        Mesh->SetSimulatePhysics(true);
        // 设置复制模式
        // Mesh->BodyInstance.PhysicsReplicationMode =
        //     EPhysicsReplicationMode::PredictiveInterpolation;
    }
    
    // bReplicateMovement 背后的原理：
    // FRepMovement 结构体通过 Actor 的 ReplicatedMovement 属性广播
    // 包含：Location / Rotation / LinearVelocity / AngularVelocity
    
    // 手动控制同步频率（默认跟随 NetUpdateFrequency）
    // 对于快速运动物体：NetUpdateFrequency = 60.f（高频）
    // 对于缓慢物体（静止桌子）：NetUpdateFrequency = 5.f（低频，省带宽）
};
```

## 二、物理睡眠时停止同步

```cpp
// 节省带宽：物体静止时停止复制
void Tick(float Dt)
{
    // 只有 Server 执行
    if (HasAuthority() && Mesh->IsSimulatingPhysics())
    {
        bool bSleeping = Mesh->RigidBodyIsAsleep();
        if (bSleeping)
        {
            // 物体已静止，降低复制频率
            NetUpdateFrequency = 1.f;
        }
        else
        {
            // 物体运动中，高频复制
            NetUpdateFrequency = 30.f;
        }
    }
}
```
