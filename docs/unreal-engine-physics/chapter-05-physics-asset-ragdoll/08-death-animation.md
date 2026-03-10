# 5.8 死亡动画过渡

> **难度**：⭐⭐⭐⭐☆

## 一、死亡过渡方案对比

```
方案 A：立即布娃娃（硬切）
  优点：实现最简单
  缺点：不自然，尤其站立死亡时突然软倒
  适用：快节奏游戏、大量 NPC 死亡场景

方案 B：死亡动画 → 布娃娃（延迟切换）
  优点：有自然的死亡倒地动作
  缺点：动画过渡时间窗口期无法受力
  适用：主要角色死亡、剧情死亡

方案 C：死亡动画 + 物理混合（推荐）
  优点：既有死亡动作，又能受到爆炸/子弹力影响
  缺点：实现最复杂
  适用：AAA 游戏标准

方案 D：Death Montage → PoseSnapshot → 布娃娃
  死亡 Montage 播放 → 在某帧快照姿势 → 从该姿势过渡到布娃娃
  → 衔接最流畅
```

## 二、方案 C 实现（死亡动画+物理混合）

```cpp
void OnCharacterDeath()
{
    bIsDead = true;
    
    // 1. 播放死亡动画（保留一定时间给动画表现）
    PlayAnimMontage(DeathMontage);
    
    // 2. 立即开启物理但权重为 0（准备好物理状态）
    GetMesh()->SetAllBodiesSimulatePhysics(true);
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->PhysicsBlendWeight = 0.f;
    
    // 3. 动画播放 0.3s 后开始混入物理权重
    FTimerHandle PhysicsTimer;
    GetWorld()->GetTimerManager().SetTimer(PhysicsTimer,
        FTimerDelegate::CreateLambda([this]()
        {
            // 在 0.4s 内从动画过渡到布娃娃
            StartRagdollBlend(0.4f);
        }), 0.3f, false);
    
    // 4. 禁用角色移动
    GetCharacterMovement()->DisableMovement();
    
    // 5. 延迟关闭胶囊（等动画做完）
    FTimerHandle CapsuleTimer;
    GetWorld()->GetTimerManager().SetTimer(CapsuleTimer,
        FTimerDelegate::CreateLambda([this]()
        {
            GetCapsuleComponent()->SetCollisionEnabled(
                ECollisionEnabled::NoCollision);
        }), 0.5f, false);
}
```

## 三、尸体管理

```
死亡后的尸体处理：

// 一段时间后消失（防止太多尸体积累）
void ScheduleBodyCleanup(float Delay = 10.f)
{
    FTimerHandle CleanupTimer;
    GetWorld()->GetTimerManager().SetTimer(CleanupTimer,
        FTimerDelegate::CreateLambda([this]()
        {
            // 方案 A：直接销毁
            Destroy();
            
            // 方案 B：淡出后销毁（更优雅）
            if (UMaterialInstanceDynamic* MI =
                GetMesh()->CreateDynamicMaterialInstance(0))
            {
                // 逐渐降低 Opacity
                // ... 配合 Timeline 实现淡出
            }
        }), Delay, false);
}

// 物理 LOD：距离过远的尸体改为 Kinematic
void OptimizeDistantBodies()
{
    float DistToPlayer = FVector::Distance(
        GetActorLocation(),
        UGameplayStatics::GetPlayerPawn(this, 0)->GetActorLocation());
    
    if (DistToPlayer > 2000.f && bIsDead)
    {
        GetMesh()->SetSimulatePhysics(false);
        // 让尸体保持当前姿势静止
    }
}
```

## 四、延伸阅读

- 📄 [5.9 性能优化](./09-performance-optimization.md)
