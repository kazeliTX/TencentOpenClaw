# 9.8 延迟补偿与动画

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 30 分钟

---

## 一、延迟补偿的概念

```
问题（FPS 游戏场景）：
  Client A（RTT=100ms）在 T=1000ms 开枪，瞄准了 Client B
  但 Client A 看到的 Client B 是 50ms 前的位置（历史位置）
  服务端在 T=1050ms 收到射击请求
  此时服务端的 Client B 已经向前移动了 50ms
  
  如果直接检测：Client A 明明打中了，但服务端认为打偏了！

延迟补偿（Lag Compensation）：
  服务端记录所有角色的历史位置（Hitbox 历史快照）
  收到射击请求时，回溯到 Client A 开枪时刻的历史状态
  在历史状态下检测命中
  → 对 Client A 公平（瞄哪打哪）
```

---

## 二、延迟补偿与动画的关系

```
延迟补偿需要记录的不仅是位置，还有 Hitbox：
  Hitbox 由骨骼驱动（Physics Asset 中的 Body）
  骨骼位置由动画决定
  → 动画状态也需要历史记录！

实际简化（大多数游戏的做法）：
  不记录完整骨骼变换历史（太贵）
  只记录 Capsule/Box Hitbox 的历史位置
  射击检测用简化 Hitbox（头部球体 + 躯干胶囊 + 腿部胶囊）
  → 精度够用，开销可接受

GAS（Gameplay Ability System）+ 延迟补偿：
  UE 的 GAS 插件内置基本的延迟补偿支持
  大型项目推荐基于 GAS 构建
```

---

## 三、简单 Hitbox 历史记录

```cpp
// 简化的位置历史记录器
struct FCharacterHistoryFrame
{
    float    Timestamp;
    FVector  Location;
    FQuat    Rotation;
    FVector  HeadLocation;    // 头部骨骼位置（用于爆头检测）
};

class ULagCompensationComponent : public UActorComponent
{
    // 保存最近 500ms 的历史帧（约 10~30 帧）
    TArray<FCharacterHistoryFrame> HistoryFrames;
    float MaxHistoryTime = 0.5f;

    void TickComponent(float DeltaTime, ...) override
    {
        ACharacter* C = Cast<ACharacter>(GetOwner());
        if (!C || !C->HasAuthority()) return;  // 只在服务端记录

        FCharacterHistoryFrame Frame;
        Frame.Timestamp    = GetWorld()->GetTimeSeconds();
        Frame.Location     = C->GetActorLocation();
        Frame.Rotation     = C->GetActorQuat();
        Frame.HeadLocation = C->GetMesh()->GetBoneLocation(FName("head"));

        HistoryFrames.Insert(Frame, 0);

        // 清理过期帧
        float CutoffTime = Frame.Timestamp - MaxHistoryTime;
        HistoryFrames.RemoveAll([CutoffTime](const FCharacterHistoryFrame& F){
            return F.Timestamp < CutoffTime;
        });
    }

    // 获取指定时刻的历史状态（用于延迟补偿射线检测）
    bool GetHistoryAtTime(float TargetTime, FCharacterHistoryFrame& OutFrame)
    {
        for (int32 i = 0; i < HistoryFrames.Num() - 1; i++)
        {
            if (HistoryFrames[i].Timestamp >= TargetTime &&
                HistoryFrames[i+1].Timestamp <= TargetTime)
            {
                float Alpha = (TargetTime - HistoryFrames[i+1].Timestamp) /
                              (HistoryFrames[i].Timestamp - HistoryFrames[i+1].Timestamp);
                OutFrame.Location     = FMath::Lerp(HistoryFrames[i+1].Location, HistoryFrames[i].Location, Alpha);
                OutFrame.HeadLocation = FMath::Lerp(HistoryFrames[i+1].HeadLocation, HistoryFrames[i].HeadLocation, Alpha);
                return true;
            }
        }
        return false;
    }
};
```

---

## 四、延伸阅读

- 🔗 [GAS 延迟补偿](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-ability-system-for-unreal-engine)
- 🔗 [网络物理延迟补偿](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-prediction-in-unreal-engine)
