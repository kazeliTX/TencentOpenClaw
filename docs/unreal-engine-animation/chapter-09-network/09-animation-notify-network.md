# 9.9 AnimNotify 网络触发

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、AnimNotify 的网络问题

```
AnimNotify 默认只在本地触发：
  每个客户端独立播放动画 → 独立触发 AnimNotify
  
  问题：AnimNotify 触发的逻辑（如伤害判定、生成粒子）
        如果在每个客户端都执行 → 重复执行！

解决方案：
  纯视觉效果（粒子/音效）：每个客户端独立触发 ✅（正确！）
  游戏逻辑（伤害/道具生成）：只在服务端执行
  
  判断规则：
    if (HasAuthority())  ← 只在服务端执行游戏逻辑
        DealDamage();
    
    // 粒子/音效不需要判断，直接在每个客户端执行即可
    SpawnParticleEffect();
```

---

## 二、服务端 AnimNotify 触发模式

```cpp
// 自定义 AnimNotify：只在服务端触发伤害判定
UCLASS()
class UAN_MeleeHit : public UAnimNotify
{
    GENERATED_BODY()
    
    virtual void Notify(USkeletalMeshComponent* MeshComp,
                        UAnimSequenceBase* Animation) override
    {
        AMyCharacter* Character = Cast<AMyCharacter>(MeshComp->GetOwner());
        if (!Character) return;
        
        // 伤害判定：只在服务端执行
        if (Character->HasAuthority())
        {
            // 执行近战攻击的碰撞检测和伤害逻辑
            Character->ExecuteMeleeHitCheck();
        }
        
        // 音效/粒子：每个客户端都执行（本地效果）
        if (MeshComp->GetWorld())
        {
            UGameplayStatics::SpawnSoundAtLocation(
                MeshComp, SwingSound, MeshComp->GetComponentLocation());
        }
    }
    
    UPROPERTY(EditAnywhere, Category="Sound")
    TObjectPtr<USoundBase> SwingSound;
};
```

---

## 三、AnimNotifyState 的网络注意

```cpp
// AnimNotifyState（有时长的 Notify）：Begin/Tick/End 都要注意

UCLASS()
class UANS_WeaponTrace : public UAnimNotifyState
{
    GENERATED_BODY()

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation, float TotalDuration) override
    {
        AMyCharacter* C = Cast<AMyCharacter>(MeshComp->GetOwner());
        if (C && C->HasAuthority())
            C->StartWeaponTrace();  // 服务端开启武器碰撞检测
    }

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation) override
    {
        AMyCharacter* C = Cast<AMyCharacter>(MeshComp->GetOwner());
        if (C && C->HasAuthority())
            C->StopWeaponTrace();   // 服务端关闭武器碰撞检测
    }
    
    // Tick：谨慎使用，高频执行，服务端开销大
    // 推荐改为：Begin 时 SetTimer，End 时 ClearTimer
};
```

---

## 四、跨网络触发特效的最佳实践

```
总结：AnimNotify 的职责分离

触发内容                 执行方式
─────────────────────────────────────────────────────────
伤害判定/逻辑           HasAuthority() 保护，服务端专属
游戏状态变更            HasAuthority() 保护 + Replicated 属性同步
粒子特效                每个客户端独立执行（无需同步）
音效                    每个客户端独立执行（无需同步）
网络 RPC 触发           不推荐在 AnimNotify 中直接调用 RPC
                        （Notify 触发时机不保证同步）

黄金法则：
  AnimNotify = 动画时间轴上的钩子（hook）
  不要在 AnimNotify 中实现核心游戏逻辑
  AnimNotify → 调用 Character 方法 → Character 处理权限判断
```

---

## 五、延伸阅读

- 🔗 [AnimNotify 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-notifies-in-unreal-engine)
- 🔗 [RPC 与权限文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-procedure-calls-in-unreal-engine)
