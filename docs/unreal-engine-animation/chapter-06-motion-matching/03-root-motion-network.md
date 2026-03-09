# 6.3 Root Motion 网络同步

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、Root Motion 网络同步的挑战

```
普通 CharacterMovement 网络同步（无 Root Motion）：
  服务器计算权威位置
  客户端预测 + 服务器校正（UE 的 CMC 标准流程）
  工作良好
  
Root Motion 网络同步的问题：
  Root Motion 位移来自动画数据（每帧的根骨骼位移）
  问题：客户端和服务器的动画可能在不同帧（网络延迟）
  → 位移数据不同步 → 位置不同步 → 错误的校正
```

---

## 二、UE 的 Root Motion 网络处理机制

```
UE CharacterMovement 的 Root Motion 同步机制：

服务器端：
  1. 角色播放 Montage（服务器调用）
  2. AnimInstance 每帧提取 Root Motion 位移
  3. CMC.PerformMovement() 使用该位移
  4. 通过 Replication 将 Montage 信息广播到所有客户端

客户端（本地玩家）：
  1. 收到 Montage RPC 或本地触发播放
  2. 预测性地应用 Root Motion（无需等待服务器确认）
  3. 服务器定期发来权威位置 → 客户端做平滑校正

客户端（非本地，观察者）：
  1. 收到服务器 Montage RPC
  2. 重放 Montage（可能有延迟）
  3. 无预测，直接模拟
```

---

## 三、服务器权威 Root Motion（推荐方案）

```cpp
// 推荐：在服务器上播放 Montage，通过 Multicast 同步到客户端
UFUNCTION(Server, Reliable)
void Server_PlayRollMontage(FVector InputDirection)
{
    // 服务器：旋转角色到输入方向
    FRotator NewRot = InputDirection.Rotation();
    SetActorRotation(NewRot);
    
    // 服务器播放 Montage
    float Duration = PlayAnimMontage(RollMontage);
    
    if (Duration > 0.f)
    {
        // 广播到所有客户端（包括本地）
        Multicast_PlayRollMontage(InputDirection);
    }
}

UFUNCTION(NetMulticast, Reliable)
void Multicast_PlayRollMontage(FVector InputDirection)
{
    if (!IsLocallyControlled()) // 只在非本地控制端播放（本地已经预测了）
    {
        SetActorRotation(InputDirection.Rotation());
        PlayAnimMontage(RollMontage);
    }
}

// 本地：立即预测（不等服务器）
void AMyCharacter::Roll()
{
    // 本地预测
    PlayAnimMontage(RollMontage);
    // 发送到服务器
    Server_PlayRollMontage(GetVelocity().GetSafeNormal());
}
```

---

## 四、Root Motion 校正（Correction）

```
问题：本地预测与服务器权威位置不同步时的校正

UE 默认校正：
  CMC 将角色传送到服务器权威位置（可能有明显跳变）

平滑校正方案：
  不立即传送，而是用 VInterpTo 平滑移动到权威位置

配置：
  CharacterMovementComponent:
    NetworkSmoothingMode: Exponential（默认）
    → 自动平滑 Root Motion 位置校正
    → 比 Linear 更自然，推荐保持默认
    
  NetworkMaxSmoothNetUpdateDist: 92cm（超过此距离强制传送）
  NetworkNoSmoothUpdateDist:     140cm（超过此距离不平滑直接传送）
```

---

## 五、延伸阅读

- 📄 [代码示例：Root Motion 网络控制器](./code/01_root_motion_controller.cpp)
- 🔗 [Character Movement 网络同步文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-networked-movement-in-the-character-movement-component)
- 🔗 [Root Motion 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/root-motion-in-unreal-engine)
