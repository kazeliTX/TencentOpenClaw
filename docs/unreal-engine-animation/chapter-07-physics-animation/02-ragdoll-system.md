# 7.2 布娃娃系统：触发、恢复、过渡

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 40 分钟

---

## 一、布娃娃系统完整架构

```
布娃娃状态机：

    [正常动画]
         │  触发条件（死亡/击晕/爆炸冲击）
         ▼
    [切换为布娃娃]
      SetSimulatePhysics(true)
      禁用 CharacterMovement
      禁用 Capsule 碰撞
         │
         ▼
    [物理模拟中]
      所有骨骼由 Chaos 物理引擎驱动
      持续检测：速度是否趋近于零（角色静止）
         │  静止条件满足（速度 < 阈值）
         ▼
    [恢复准备]
      捕获当前物理姿势（Pose Snapshot）
         │
         ▼
    [动画恢复过渡]
      SetSimulatePhysics(false)
      播放起立动画（Get Up Montage）
      Blend：Snapshot Pose → 起立动画（Alpha 0→1）
         │  动画结束
         ▼
    [回到正常动画]
```

---

## 二、触发布娃娃

```cpp
void AMyCharacter::EnableRagdoll(FVector ImpulseVelocity)
{
    // 1. 停止当前动画
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
        Anim->Montage_StopAll(0.1f);

    // 2. 禁用胶囊体（不再阻挡）
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // 3. 禁用 CharacterMovement
    GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->StopMovementImmediately();

    // 4. 开启物理模拟
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetAllBodiesBelowSimulatePhysics(NAME_None, true, true);

    // 5. 施加冲量（如爆炸推力）
    if (!ImpulseVelocity.IsNearlyZero())
        GetMesh()->AddImpulse(ImpulseVelocity, NAME_None, true); // true = velocity change

    bIsRagdoll = true;
    
    // 6. 启动静止检测计时
    GetWorldTimerManager().SetTimer(
        RagdollStillCheckTimer,
        this, &AMyCharacter::CheckRagdollStill,
        0.5f, true); // 每 0.5s 检查一次
}
```

---

## 三、静止检测与恢复触发

```cpp
void AMyCharacter::CheckRagdollStill()
{
    if (!bIsRagdoll) return;

    // 获取骨盆骨骼速度（代表整体运动）
    FVector PelvisVelocity = GetMesh()->GetBoneLinearVelocity(FName("pelvis"));
    float Speed = PelvisVelocity.Size();

    if (Speed < 10.0f) // 速度低于阈值 → 认为静止
    {
        StillFrameCount++;
        if (StillFrameCount >= 3) // 连续 3 次检查都静止
        {
            GetWorldTimerManager().ClearTimer(RagdollStillCheckTimer);
            BeginRagdollRecovery();
        }
    }
    else
    {
        StillFrameCount = 0; // 重置计数
    }
}

void AMyCharacter::BeginRagdollRecovery()
{
    // 1. 捕获当前布娃娃姿势（关键：在停止物理之前！）
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
        Anim->SnapshotPose(RagdollSnapshot);

    // 2. 判断角色是趴着还是躺着（决定用哪个起立动画）
    FVector PelvisUp = GetMesh()->GetBoneAxis(FName("pelvis"), EBoneAxis::BA_Z);
    bool bFaceDown = FVector::DotProduct(PelvisUp, FVector::UpVector) < 0.0f;
    
    UAnimMontage* GetUpMontage = bFaceDown ? GetUpFromFront : GetUpFromBack;

    // 3. 关闭物理
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
    
    // 4. 重新启用移动
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // 5. 同步胶囊体到骨盆位置
    FVector PelvisLocation = GetMesh()->GetBoneLocation(FName("pelvis"));
    SetActorLocation(PelvisLocation + FVector(0, 0, GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

    // 6. 播放起立 Montage（AnimGraph 中 Snapshot Pose 和起立动画混合）
    bIsRagdoll = false;
    bIsGettingUp = true;
    PlayAnimMontage(GetUpMontage);
}
```

---

## 四、AnimGraph 中的布娃娃过渡

```
布娃娃恢复时的 AnimGraph 配置：

[Snapshot Pose 节点]
    Snapshot: RagdollSnapshot（AnimInstance 变量）
        │
[Blend]
    A: Snapshot Pose    （布娃娃姿势，Alpha=1→0）
    B: Get Up Montage   （起立动画，Alpha=0→1）
    Alpha: GetUpBlendAlpha（由时间曲线驱动，从 0 线性到 1）
        │
[Output Pose]

Alpha 控制：
  起立动画播放时间 / 总时长 = 当前混合 Alpha
  → 随起立动画进度，逐渐从布娃娃姿势过渡到起立动画
```

---

## 五、延伸阅读

- 📄 [代码示例：完整布娃娃控制器](./code/01_ragdoll_controller.cpp)
- 🔗 [Pose Snapshot 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/pose-snapshot-in-unreal-engine)
- 🔗 [Ragdoll 物理设置](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-asset-editor-in-unreal-engine)
