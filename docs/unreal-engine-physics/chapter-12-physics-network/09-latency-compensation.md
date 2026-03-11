# 12.9 延迟补偿

> **难度**：⭐⭐⭐⭐⭐

## 一、延迟补偿原理

```
延迟补偿（Lag Compensation）：
  玩家 A 在本地看到玩家 B 在位置 P1 开枪
  但服务端收到时，玩家 B 实际已移动到 P2
  
  没有延迟补偿：子弹在服务端检测 P2 → Miss（明明打中了）
  有延迟补偿：服务端回滚到 A 开枪那一刻 → 检测 P1 → Hit（公平）

UE 内置延迟补偿：
  CharacterMovementComponent 内置基本延迟补偿
  物理对象需要手动实现
  
物理对象延迟补偿框架：
  服务端保存过去 N ms 的物理快照（位置/旋转历史）
  收到客户端请求时：
    1. 根据客户端 RTT 计算"客户端那时候"的时间戳
    2. 回滚服务端物理到该时间戳
    3. 执行碰撞检测/命中判定
    4. 恢复物理到当前状态
```

## 二、快照历史实现

```cpp
USTRUCT()
struct FPhysicsSnapshot
{
    GENERATED_BODY()
    float     Timestamp;
    FVector   Location;
    FQuat     Rotation;
    FVector   LinearVelocity;
    FVector   AngularVelocity;
};

UCLASS()
class APhysicsActorWithLC : public AActor
{
    GENERATED_BODY()
public:
    TArray<FPhysicsSnapshot> SnapshotHistory;
    static const int32 MaxHistory = 60; // 保存 60 帧历史（约 1 秒）

    virtual void Tick(float Dt) override
    {
        Super::Tick(Dt);
        if (HasAuthority()) RecordSnapshot();
    }

    void RecordSnapshot()
    {
        FPhysicsSnapshot S;
        S.Timestamp       = GetWorld()->GetTimeSeconds();
        S.Location        = GetActorLocation();
        S.Rotation        = GetActorQuat();
        if (UPrimitiveComponent* PC = GetRootPrimitiveComponent())
        {
            S.LinearVelocity  = PC->GetPhysicsLinearVelocity();
            S.AngularVelocity = PC->GetPhysicsAngularVelocityInRadians();
        }
        SnapshotHistory.Insert(S, 0);
        if (SnapshotHistory.Num() > MaxHistory)
            SnapshotHistory.SetNum(MaxHistory);
    }

    // 获取指定时间戳的插值快照
    FPhysicsSnapshot GetSnapshotAtTime(float TargetTime) const
    {
        for (int32 i = 0; i + 1 < SnapshotHistory.Num(); i++)
        {
            const FPhysicsSnapshot& A = SnapshotHistory[i+1];
            const FPhysicsSnapshot& B = SnapshotHistory[i];
            if (A.Timestamp <= TargetTime && TargetTime <= B.Timestamp)
            {
                float Alpha = (TargetTime - A.Timestamp) / (B.Timestamp - A.Timestamp);
                FPhysicsSnapshot Result;
                Result.Timestamp = TargetTime;
                Result.Location  = FMath::Lerp(A.Location, B.Location, Alpha);
                Result.Rotation  = FQuat::Slerp(A.Rotation, B.Rotation, Alpha);
                return Result;
            }
        }
        return SnapshotHistory.IsEmpty() ?
            FPhysicsSnapshot{} : SnapshotHistory.Last();
    }
};
```
