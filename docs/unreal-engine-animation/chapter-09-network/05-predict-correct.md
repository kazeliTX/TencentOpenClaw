# 9.5 客户端预测与服务端校正

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 35 分钟

---

## 一、客户端预测的必要性

```
网络延迟（RTT）通常 50~200ms：
  如果等服务端确认再执行移动 → 玩家感受到 25~100ms 延迟
  → 操作感极差，现代游戏不可接受

解决方案：客户端预测（Client-Side Prediction）
  客户端立即执行移动（不等服务端）
  同时发送移动指令到服务端
  服务端验证 → 如果结果不同 → 服务端校正客户端位置
  
动画受到的影响：
  预测阶段：动画基于预测位置/速度（流畅）
  校正阶段：位置被"拉回"正确位置 → 可能导致动画跳变
```

---

## 二、CMC 内置的预测校正

```
CharacterMovementComponent 已内置处理：

预测流程（每帧）：
  Client：
    保存当前输入（FSavedMove）
    预测执行移动
    发送 ServerMove RPC（包含输入 + 预测结果）
    
  Server：
    重新执行移动逻辑
    如果服务端结果 ≠ 客户端预测：发送 ClientAdjustPosition
    
  Client 收到校正：
    强制将位置设置为服务端权威位置
    回放（Replay）后续未确认的输入（重新预测）

动画的处理：
  校正发生时，位置突变 → 动画可能出现跳帧
  CMC 使用平滑插值（ClientSmoothCorrection）缓解这个问题：
    p.NetEnableMoveSmoothingParam 1   ← 开启移动平滑
    p.SmoothNetUpdateRate 12          ← 平滑速率（越高越快收敛）
```

---

## 三、动画预测的高级处理

```
技能/攻击预测（游戏层面，非 CMC）：

架构：
  Client：
    输入 → 本地立即播放攻击 Montage（预测）
    发送攻击请求到服务端
    
  Server：
    验证攻击（冷却、距离、状态）
    
    合法 → 广播攻击 Montage（其他客户端播放）
            + 计算伤害
    
    非法 → 发送 RPC 告知客户端取消（ClientCancelAttack）

  Client 收到 Cancel：
    停止本地播放的 Montage
    播放取消动画（如被打断）

处理不一致：
  本地播放 vs 服务端拒绝 → 短暂的"幻象动画"（< 150ms）
  → 大多数游戏直接接受这个不一致（玩家几乎不会注意到）
  → 严格同步的游戏（竞技射击）需要完整的 Rollback 系统
```

---

## 四、预测失误的可视化处理

```
当服务端校正导致位置跳变时，对动画做平滑处理：

// 监听服务端校正事件
void AMyCharacter::OnCorrectionReceived(FVector CorrectionDelta)
{
    // 如果校正量很小（< 5cm）：直接接受，动画基本无感知
    if (CorrectionDelta.SizeSquared() < 25.f) return;
    
    // 较大校正：触发"踉跄"动画遮盖位置跳变
    if (CorrectionDelta.SizeSquared() < 10000.f) // < 1m
    {
        // 播放一个短暂的 stumble 动画（遮盖校正感知）
        PlayAnimMontage(StumbleMontage, 1.5f); // 加速播放
    }
    else
    {
        // 极大校正（作弊或严重延迟）：直接传送
        TeleportTo(GetActorLocation(), GetActorRotation());
    }
}
```

---

## 五、延伸阅读

- 📄 [9.6 Root Motion 网络同步](./06-rootmotion-network.md)
- 🔗 [CMC 网络文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-networked-movement-in-the-character-movement-component)
