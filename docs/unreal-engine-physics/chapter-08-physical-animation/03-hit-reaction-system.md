# 8.3 命中反应系统

> **难度**：⭐⭐⭐⭐⭐

## 一、基于 Physical Animation 的命中反应

```cpp
void HandleHit(const FHitResult& Hit, float ImpulseMag)
{
    FName BoneName = Hit.BoneName;
    
    // 1. 在命中骨骼施加冲量
    GetMesh()->AddImpulseAtLocation(
        -Hit.ImpactNormal * ImpulseMag,
        Hit.ImpactPoint,
        BoneName);
    
    // 2. 临时降低 Physical Animation 追随强度（更明显的物理反应）
    PhysAnim->SetStrengthMultiplier(0.1f, BoneName);
    
    // 3. 延迟恢复（0.5s 后回到正常追随强度）
    FTimerHandle T;
    GetWorld()->GetTimerManager().SetTimer(T,
        FTimerDelegate::CreateLambda([this, BoneName]() {
            PhysAnim->SetStrengthMultiplier(1.0f, BoneName);
        }), 0.5f, false);
}
```

## 二、多部位命中叠加

```
多次命中时叠加效果：
  第一发：右肩摆动
  第二发（连续）：右肩摆动 + 身体向右倾
  第三发：可能触发完整布娃娃（累积伤害）
  
实现：
  维护命中计数器和累积冲量
  当累积超过阈值 → 触发完整 Ragdoll
```
