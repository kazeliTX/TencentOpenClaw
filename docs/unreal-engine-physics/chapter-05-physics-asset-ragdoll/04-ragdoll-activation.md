# 5.4 Ragdoll 激活流程

> **难度**：⭐⭐⭐⭐☆

## 一、标准激活步骤

```
完整 Ragdoll 激活顺序（顺序很重要！）：

Step 1：禁用 CharacterMovement
  GetCharacterMovement()->DisableMovement();
  GetCharacterMovement()->StopMovementImmediately();

Step 2：关闭胶囊碰撞
  GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

Step 3：解除 Mesh 附属
  GetMesh()->DetachFromComponent(
      FDetachmentTransformRules::KeepWorldTransform);
  // 保持世界变换，让 Mesh 原地激活物理

Step 4：切换碰撞 Profile
  GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
  // Ragdoll Profile：
  //   对象类型 = PhysicsBody
  //   Block WorldStatic/WorldDynamic
  //   Ignore Pawn（不与其他角色碰撞）

Step 5：开启物理模拟
  GetMesh()->SetSimulatePhysics(true);

Step 6：继承速度（让布娃娃保持角色当前速度）
  FVector Vel = GetVelocity();
  GetMesh()->SetPhysicsLinearVelocity(Vel, false, NAME_None);
  // 注意：SetSimulatePhysics(true) 之后才能设速度

Step 7：根骨骼对齐（可选）
  // 让 Actor Location 跟随物理根骨骼
  // 通常在 Tick 中更新：
  // SetActorLocation(GetMesh()->GetBoneLocation("pelvis"));
```

## 二、武器/装备的处理

```
激活 Ragdoll 时，武器需要单独处理：

方案 A：武器随布娃娃手部运动（焊接）
  EquippedWeapon->AttachToComponent(
      GetMesh(),
      FAttachmentTransformRules::SnapToTargetIncludingScale,
      FName("hand_r"));
  EquippedWeapon->GetMesh()->SetCollisionEnabled(
      ECollisionEnabled::NoCollision); // 武器不参与碰撞

方案 B：武器脱手飞出（独立物理）
  EquippedWeapon->DetachFromActor(
      FDetachmentTransformRules::KeepWorldTransform);
  EquippedWeapon->GetMesh()->SetSimulatePhysics(true);
  // 继承手部速度
  FVector HandVel = GetMesh()->GetPhysicsLinearVelocity("hand_r");
  EquippedWeapon->GetMesh()->SetPhysicsLinearVelocity(HandVel);
  EquippedWeapon->SetLifeSpan(8.f);

方案 C：武器消失（最简单）
  EquippedWeapon->SetActorHiddenInGame(true);
  EquippedWeapon->GetMesh()->SetCollisionEnabled(
      ECollisionEnabled::NoCollision);
```

## 三、常见激活错误

```
问题：激活后 Mesh 瞬移到地板下
  原因：DetachFromComponent 时变换计算错误
  修复：
    DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    ↑ 确保用 KeepWorldTransform（不是 KeepRelativeTransform）

问题：激活后角色飞出地图
  原因：之前积累的 CharacterMovement 速度传递给了物理体
  修复：
    GetCharacterMovement()->StopMovementImmediately();
    // 先停运动，再激活物理

问题：Ragdoll 穿墙
  原因：速度过大，子步长不够
  修复：
    Mesh->SetCCDEnabled(true);
    或降低速度：FVector Vel = GetVelocity(); Vel = Vel.GetClampedToMaxSize(1000.f);
    GetMesh()->SetPhysicsLinearVelocity(Vel);

问题：布娃娃腿部卡在地板上
  原因：胶囊关闭时 Mesh 在地板以下
  修复：
    // 先让 Mesh 与胶囊底部对齐
    FVector CapsuleBottom = GetActorLocation();
    CapsuleBottom.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    GetMesh()->SetWorldLocation(CapsuleBottom - FVector(0,0,90)); // 骨骼偏移
```

## 四、延伸阅读

- 📄 [5.5 布娃娃动画混合](./05-ragdoll-blending.md)
- 📄 [代码示例：完整 Ragdoll 系统](./code/01_ragdoll_system.cpp)
