# 9.6 Root Motion 网络同步

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 30 分钟

---

## 一、Root Motion 网络同步挑战

```
Root Motion 是动画驱动角色位移（而非 CMC 驱动）
这与 CMC 的预测系统产生冲突：

CMC 预测系统假设：位移由玩家输入决定（可预测）
Root Motion 的问题：位移由动画帧决定（与输入无关）

后果：
  Root Motion 的位移不能被 CMC 正常预测
  → 服务端/客户端的 Root Motion 可能产生位置不一致
  → 校正频繁 → 画面抖动
```

---

## 二、UE Root Motion 网络同步机制

```
UE 内置的处理方式（AnimRootMotionTranslationScale）：

Server-Auth Root Motion：
  服务端执行 Root Motion，计算最终位置
  通过 CMC 的位置同步广播给客户端
  客户端收到位置 → 插值移动到该位置
  
  问题：客户端看到延迟（RTT/2）的运动
        攻击动画的"冲刺"部分会有明显延迟感

推荐解决方案（大多数游戏采用）：
  模式：Root Motion from Montages Only（见第六章）
  
  Locomotion：CMC 驱动（完整预测，无延迟）
  技能/攻击：Montage Root Motion
    → 客户端：立即播放 Montage（预测 Root Motion）
    → 服务端：验证后广播 Montage
    → 不一致：< RTT/2 的短暂偏差，通常可接受
```

---

## 三、Root Motion 网络配置

```cpp
// 推荐配置
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // 服务端：完整物理
    // 客户端：SimulatedProxy 不执行 Root Motion（避免与服务端不同步）
    if (!HasAuthority() && !IsLocallyControlled())
    {
        // SimulatedProxy：禁用 Root Motion（由位置插值代替）
        GetMesh()->RootMotionMode = ERootMotionMode::IgnoreRootMotion;
    }
    else
    {
        // Server + AutProxy：完整 Root Motion
        GetMesh()->RootMotionMode = ERootMotionMode::RootMotionFromMontagesOnly;
    }
}
```

---

## 四、Root Motion Montage 的同步时序

```
时序图（100ms RTT 示例）：

T=0ms    Client：按下攻击，本地立即播放 Montage（Root Motion 开始）
T=0ms    Client：发送 ServerAttack RPC
T=50ms   Server：收到 RPC，播放 Montage（服务端 Root Motion 开始）
T=50ms   Server：广播 MulticastAttack
T=100ms  Other Clients：收到广播，播放 Montage

结果：
  本地玩家：零延迟（最流畅）
  服务端：50ms 延迟（安全）
  其他玩家看到：100ms 延迟（可接受）

位置偏差：
  Server Root Motion 比 Client 晚 50ms 开始
  → 50ms × 移动速度 ≈ 50ms × 500cm/s = 25cm 偏差
  CMC 的平滑校正会在约 200ms 内消除这个偏差
```

---

## 五、延伸阅读

- 🔗 [Root Motion 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/root-motion-in-unreal-engine)
- 🔗 [CMC Root Motion 处理](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-networked-movement-in-the-character-movement-component)
