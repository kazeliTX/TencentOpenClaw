# 9.4 Montage 网络同步

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、Montage 同步的核心问题

```
Montage 是一次性事件（攻击/技能/受击）
不能用 Replicated 属性同步（属性同步有延迟，可能被跳过）
必须用 RPC（远程过程调用）可靠触发

错误做法：
  UPROPERTY(Replicated) UAnimMontage* CurrentMontage;
  → 属性同步不保证触发时机，客户端可能错过播放

正确做法：
  用 NetMulticast RPC 广播"播放此 Montage"事件
  每个客户端独立播放对应的 Montage
```

---

## 二、标准 Montage 同步架构

```cpp
// AMyCharacter.cpp

// 客户端按下攻击键
void AMyCharacter::LocalAttack()
{
    if (IsLocallyControlled())
    {
        // 本地立即播放（无延迟，流畅感）
        PlayAnimMontage(AttackMontage);
        // 同时告诉服务端
        ServerPlayMontage(AttackMontage);
    }
}

// 服务端 RPC：验证并广播
UFUNCTION(Server, Reliable)
void ServerPlayMontage(UAnimMontage* Montage);
void AMyCharacter::ServerPlayMontage_Implementation(UAnimMontage* Montage)
{
    // 服务端验证（防作弊：检查 CD、状态合法性等）
    if (!CanAttack()) return;
    
    // 服务端自己也播放
    PlayAnimMontage(Montage);
    
    // 广播给所有其他客户端
    MulticastPlayMontage(Montage);
}

// 多播 RPC：在所有客户端播放
UFUNCTION(NetMulticast, Reliable)
void MulticastPlayMontage(UAnimMontage* Montage);
void AMyCharacter::MulticastPlayMontage_Implementation(UAnimMontage* Montage)
{
    // 跳过本地控制的客户端（已经在 LocalAttack 中播放了）
    if (!IsLocallyControlled())
        PlayAnimMontage(Montage);
}
```

---

## 三、Montage ID 同步（带宽优化版本）

```cpp
// 直接传 UAnimMontage* 指针会导致额外开销
// 改用 uint8 ID，减少网络数据

// 在 Character 中维护 Montage 映射表
UPROPERTY(EditAnywhere, Category="Anim")
TArray<UAnimMontage*> MontageList;  // 在编辑器中配置

UFUNCTION(Server, Reliable)
void ServerPlayMontageByID(uint8 MontageID);

UFUNCTION(NetMulticast, Reliable)
void MulticastPlayMontageByID(uint8 MontageID);

void AMyCharacter::ServerPlayMontageByID_Implementation(uint8 MontageID)
{
    if (!MontageList.IsValidIndex(MontageID)) return;
    PlayAnimMontage(MontageList[MontageID]);
    MulticastPlayMontageByID(MontageID);
}

void AMyCharacter::MulticastPlayMontageByID_Implementation(uint8 MontageID)
{
    if (!IsLocallyControlled() && MontageList.IsValidIndex(MontageID))
        PlayAnimMontage(MontageList[MontageID]);
}
// 带宽：uint8（1字节）vs UAnimMontage*（8字节）
```

---

## 四、Montage 同步的注意事项

```
1. Reliable vs Unreliable RPC
   攻击/技能 Montage：Reliable（不能丢失）
   受击 Montage（频繁触发）：可用 Unreliable（偶尔丢失可接受）
   环境交互动画：Reliable

2. 时序问题
   本地播放（即时）vs 服务端广播（有延迟）
   → 其他玩家看到的 Montage 会晚 RTT/2（50~150ms）
   → 对于攻击动画：视觉上可接受
   → 对于同步精度要求高的场景：需要延迟补偿（第9.8节）

3. Montage 打断同步
   如果服务端打断了 Montage，需要同步打断事件：
   UFUNCTION(NetMulticast, Reliable)
   void MulticastStopMontage(float BlendOutTime);
```

---

## 五、延伸阅读

- 📄 [代码示例：Montage 同步系统](./code/02_montage_replication.cpp)
- 🔗 [RPC 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/remote-procedure-calls-in-unreal-engine)
