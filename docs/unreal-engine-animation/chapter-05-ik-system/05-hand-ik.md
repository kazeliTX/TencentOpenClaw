# 5.5 手部 IK（武器握持 / 双手持枪）

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、手部 IK 的应用场景

```
手部 IK 主要解决两个问题：

1. 双手持枪 / 持武器对齐
   左手需要精确放在武器的握把位置
   武器跟随右手运动，左手 IK 跟随武器
   
2. 互动手部 IK（开门、按按钮、拾取）
   手部精确到达目标交互点
   与 Motion Warping 配合使用
```

---

## 二、双手持枪 IK 标准方案（UE5 Lyra 方案）

```
Lyra 的双手持枪 IK 架构（推荐学习）：

骨骼层级中的特殊 IK 骨骼：
  ik_hand_gun:   右手持枪的主 IK 效应器（武器绑定到这里）
  ik_hand_l:     左手 IK 效应器
  ik_hand_r:     右手 IK 效应器
  ik_hand_root:  手部 IK 的根节点

工作流程：
  1. 武器的位置/旋转跟随 ik_hand_gun 骨骼（Socket）
  2. ik_hand_gun 跟随动画中 hand_r 骨骼的变换
  3. 武器 Mesh 上定义 LeftHandSocket（左手抓握点）
  4. ik_hand_l 的目标 = LeftHandSocket 的世界位置
  5. 左臂 Two-Bone IK → ik_hand_l

数据流：
  hand_r（动画） → ik_hand_gun → 武器变换
  武器.LeftHandSocket → IK 目标 → 左臂 Two-Bone IK → hand_l
```

### AnimGraph 实现

```
AnimGraph 层级（双手持枪）：

[下半身 State Machine]
        │
[Layered Blend Per Bone（上半身）]
  Base:    ← 下半身
  Blend 0: ← [Slot: UpperBody]（Montage 攻击插入点）
              │
           [Apply Aim Offset]
              │
           [Two-Bone IK（右臂）]
             IK Bone: hand_r
             Effector: ik_hand_r 骨骼变换
              │
           [Two-Bone IK（左臂）]
             IK Bone: hand_l
             Effector: ik_hand_l 目标（来自武器 Socket）
             Alpha: LeftHandIKAlpha（可以在换弹时渐隐）
```

---

## 三、运行时计算左手 IK 目标

```cpp
void UMyAnimInstance::UpdateHandIK(float DeltaTime)
{
    if (!CachedChar) return;

    // 找到当前武器
    AActor* Weapon = CachedChar->GetCurrentWeapon();
    if (!Weapon)
    {
        LeftHandIKAlpha = FMath::FInterpTo(LeftHandIKAlpha, 0.0f, DeltaTime, 10.0f);
        return;
    }

    // 找到武器上的左手 Socket
    USkeletalMeshComponent* WeaponMesh =
        Weapon->FindComponentByClass<USkeletalMeshComponent>();
    if (!WeaponMesh) return;

    // 获取左手抓握点（武器 Mesh 上预先定义好的 Socket）
    FTransform LeftHandSocketWorld =
        WeaponMesh->GetSocketTransform(FName("LeftHandSocket"));

    // 转换到角色 Mesh 的 Component Space
    FTransform MeshWorldInverse = CachedMesh->GetComponentTransform().Inverse();
    FTransform LeftHandCS = LeftHandSocketWorld * MeshWorldInverse;

    // 平滑更新 IK 目标
    LeftHandIKTarget = FMath::VInterpTo(
        LeftHandIKTarget, LeftHandCS.GetLocation(), DeltaTime, 20.0f);
    LeftHandIKRotation = FMath::RInterpTo(
        LeftHandIKRotation, LeftHandCS.GetRotation().Rotator(), DeltaTime, 20.0f);

    // 平滑开启 IK Alpha
    LeftHandIKAlpha = FMath::FInterpTo(LeftHandIKAlpha, 1.0f, DeltaTime, 10.0f);
}
```

---

## 四、换弹时处理双手 IK

```cpp
// 换弹时，左手离开武器 → IK Alpha 渐隐
void AMyCharacter::StartReload()
{
    // 播放换弹 Montage
    PlayAnimMontage(ReloadMontage);

    // 通知 AnimInstance 禁用左手 IK
    if (UMyAnimInstance* Anim = Cast<UMyAnimInstance>(GetMesh()->GetAnimInstance()))
    {
        Anim->bDisableLeftHandIK = true;
    }
}

// 在换弹 Montage 结束后恢复
void AMyCharacter::OnReloadFinished()
{
    if (UMyAnimInstance* Anim = Cast<UMyAnimInstance>(GetMesh()->GetAnimInstance()))
    {
        Anim->bDisableLeftHandIK = false;
    }
}
```

---

## 五、延伸阅读

- 📄 [代码示例：手部 IK 控制器](./code/02_hand_ik_controller.cpp)
- 🔗 [Lyra 双手持枪 IK 分析](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-in-lyra-sample-game-in-unreal-engine)
