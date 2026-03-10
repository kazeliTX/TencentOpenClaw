# 5.9 Ragdoll 性能优化

> **难度**：⭐⭐⭐⭐☆

## 一、同时存在 Ragdoll 数量限制

```
性能预算：
  低端设备（移动端）：≤ 4 个同时活跃 Ragdoll
  主机/PC（中等）：≤ 16 个同时活跃 Ragdoll
  高端 PC：≤ 32 个同时活跃 Ragdoll（Chaos 异步物理下）

超出预算的处理策略（优先级排序）：
  1. 距离最近的保持 Ragdoll（玩家最可能看到）
  2. 超出距离阈值的 → 关闭物理，保持静止姿势
  3. 最老的（死亡时间最长的）→ 销毁/对象池回收

// RagdollManager 单例（游戏模式中管理）
class ARagdollManager
{
    static const int32 MaxActiveRagdolls = 16;
    TArray<ACharacter*> ActiveRagdolls;
    
    void RegisterRagdoll(ACharacter* Ch)
    {
        ActiveRagdolls.Add(Ch);
        if (ActiveRagdolls.Num() > MaxActiveRagdolls)
        {
            // 找到距离最远的布娃娃，关闭物理
            ACharacter* Farthest = FindFarthestFromPlayer();
            if (Farthest)
            {
                Farthest->GetMesh()->SetSimulatePhysics(false);
                ActiveRagdolls.Remove(Farthest);
            }
        }
    }
};
```

## 二、睡眠阈值优化

```
Ragdoll 静止后尽快进入 Sleep：

FBodyInstance* BI = GetMesh()->GetBodyInstance();
// 降低睡眠阈值（默认较保守）
BI->SleepFamily = ESleepFamily::Sensitive;  // 更容易入睡

// 或自定义：
// Project Settings → Physics → Sleep Threshold Multiplier → 设为 2.0~5.0（越高越容易睡）

// 手动强制睡眠（确认静止时）
void ForceSleepIfStill()
{
    FVector Vel = GetMesh()->GetPhysicsLinearVelocity();
    if (Vel.SizeSquared() < 100.f)  // 速度 < 10 cm/s
        GetMesh()->PutAllRigidBodiesToSleep();
}
```

## 三、LOD 距离裁减

```
根据距离玩家的距离降级：

距离       处理
0~500cm   完整 Ragdoll（所有 Body 模拟）
500~2000cm  Ragdoll，但跳过 Tick 更新（每 3 帧更新一次）
2000~5000cm 冻结为静止姿势（SetSimulatePhysics(false)）
>5000cm   直接隐藏/销毁

实现：
void Tick(float Dt)
{
    float Dist = DistanceToPlayer();
    if (Dist > 5000.f) { SetActorHiddenInGame(true); return; }
    if (Dist > 2000.f)
    {
        // 每 3 帧才运行物理更新
        static int32 FrameCount = 0;
        if (++FrameCount % 3 != 0) return;
    }
}
```

## 四、延伸阅读

- 📄 [5.10 Physics Asset Profiles](./10-physics-asset-profiles.md)
- 📄 [代码示例：Ragdoll 对象池](./code/03_ragdoll_pooling.cpp)
