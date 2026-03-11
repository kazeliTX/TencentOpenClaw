# 12.7 角色物理网络同步

> **难度**：⭐⭐⭐⭐⭐

## 一、Ragdoll 网络同步

```cpp
// Ragdoll 死亡同步方案：
// 只同步触发事件，每个客户端本地模拟布娃娃

UFUNCTION(NetMulticast, Reliable)
void Multicast_TriggerRagdoll(FVector DeathImpulse, FName HitBone)
{
    // 每个客户端本地触发布娃娃
    GetCharacterMovement()->DisableMovement();
    GetCapsuleComponent()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);
    GetMesh()->SetAllBodiesSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    
    // 在命中骨骼施加冲量（每个客户端相同的输入）
    GetMesh()->AddImpulseAtLocation(
        DeathImpulse, 
        GetMesh()->GetBoneLocation(HitBone),
        HitBone);
    
    // 注意：各客户端的布娃娃结果会略有不同（浮点/延迟差异）
    // 这对于布娃娃是可接受的（玩家不会盯着尸体对比）
}
```

## 二、推箱子等物理交互

```
玩家推动物体时的正确同步流程：
  1. 客户端本地：AddForce（即时手感）
  2. 服务端：Server RPC → 权威计算 → 通过 ReplicateMovement 广播
  3. 客户端：收到服务端位置 → 平滑追赶（PredictiveInterpolation）
  
  关键：只有服务端的物理计算是权威的
       客户端的本地预测只是"视觉感受"，最终以服务端为准
       
Hit Reaction（命中反应）同步：
  只需同步命中参数（骨骼/方向/力度）
  Physical Animation 响应在每个客户端本地计算
  → Multicast_OnHit(BoneName, ImpulseDir, ImpulseMag)
```
