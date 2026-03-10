# 9.2 移动同步与 CharacterMovement

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、CharacterMovement 的网络同步机制

```
CharacterMovementComponent 内置完整网络同步：

Client (AutProxy) 流程：
  1. 玩家输入 → 客户端预测移动（立即响应，流畅感）
  2. 将移动指令发送到服务端（ServerMove RPC）
  3. 服务端验证移动 → 广播权威位置
  4. 客户端收到服务端位置 → 误差校正（平滑插值回正确位置）

Server 流程：
  1. 接收客户端的 ServerMove
  2. 重新执行移动逻辑（验证合法性，防作弊）
  3. 广播位置给所有 SimulatedProxy

SimulatedProxy 流程：
  1. 收到服务端广播的位置
  2. 对位置做插值/外推预测（减少卡顿感）
  3. 驱动本地动画系统

动画关键点：
  AutProxy：GetVelocity() 返回预测速度（本地，无延迟）
  SimProxy：GetVelocity() 返回服务端同步速度（有延迟）
```

---

## 二、移动状态对动画的影响

```
CharacterMovement 自动同步的移动状态：

状态变量                  动画用途
──────────────────────────────────────────────────────
GetVelocity()             移动速度（驱动 BlendSpace）
GetCharacterMovement()
  .IsFalling()            是否在空中（跳跃/下落状态机）
  .IsMovingOnGround()     是否在地面
  .GetCurrentAcceleration() 加速度方向（倾斜动画）
  .MovementMode           移动模式（Walking/Flying/Swimming）
GetBaseTranslationOffset() 站在移动平台上的偏移量

这些值已经由 CMC 自动处理网络同步，
AnimBP 直接读取即可，不需要额外复制
```

---

## 三、自定义移动状态同步

```cpp
// 需要手动同步的额外移动状态（在 Character.h 中）
UPROPERTY(Replicated, BlueprintReadOnly, Category="Anim")
bool bIsSprinting = false;

UPROPERTY(Replicated, BlueprintReadOnly, Category="Anim")
bool bIsCrouching_Custom = false;  // 区别于 CMC 内置 Crouch

UPROPERTY(Replicated, BlueprintReadOnly, Category="Anim")
float AimPitch = 0.f;  // 瞄准俯仰角（用于 Aim Offset）

UPROPERTY(Replicated, BlueprintReadOnly, Category="Anim")
uint8 WeaponType = 0;  // 武器类型（0=无武器,1=手枪,2=步枪）

// 声明复制
void AMyCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyCharacter, bIsSprinting);
    DOREPLIFETIME(AMyCharacter, bIsCrouching_Custom);
    // 优化：只在变化时同步，减少带宽
    DOREPLIFETIME_CONDITION(AMyCharacter, AimPitch, COND_SimulatedOnly);
    DOREPLIFETIME(AMyCharacter, WeaponType);
}
```

---

## 四、AimPitch 同步优化

```cpp
// AimPitch（瞄准角度）需要高频同步，但精度可以降低
// UE 内置 RemoteViewPitch 已处理这个问题！

// 在 AnimBP 中读取任意角色的瞄准角：
float GetAimPitch(ACharacter* Character)
{
    if (Character->IsLocallyControlled())
    {
        // 本地控制：直接从控制器读取精确值
        AController* PC = Character->GetController();
        if (PC) return PC->GetControlRotation().Pitch;
    }
    else
    {
        // 远程角色：使用 UE 压缩同步的 RemoteViewPitch
        // RemoteViewPitch 是 uint8（0~255 映射到 0°~360°）
        float Pitch = (float)Character->RemoteViewPitch / 255.f * 360.f;
        // 转换为 -90 ~ 90 范围
        if (Pitch > 180.f) Pitch -= 360.f;
        return FMath::Clamp(Pitch, -90.f, 90.f);
    }
    return 0.f;
}
// 带宽：uint8（1 字节）vs FRotator float（12 字节）→ 节省 92%
```

---

## 五、延伸阅读

- 🔗 [CharacterMovement 网络文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/understanding-networked-movement-in-the-character-movement-component)
- 🔗 [网络复制文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/replication-in-unreal-engine)
