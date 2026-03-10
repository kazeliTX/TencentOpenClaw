# 5.6 起身动画（PoseSnapshot）

> **难度**：⭐⭐⭐⭐⭐

## 一、PoseSnapshot 流程

```
布娃娃 → 起身流程：

Step 1：检测方向（面朝上还是朝下）
  FVector PelvisUp = GetMesh()->GetBoneAxis("pelvis", EBoneAxis::BA_Z);
  bool bFaceUp = FVector::DotProduct(PelvisUp, FVector::UpVector) > 0.f;
  UAnimMontage* GetUpMontage = bFaceUp ? GetUpFrontMontage : GetUpBackMontage;

Step 2：保存当前布娃娃姿势
  FPoseSnapshot RagdollPose;
  GetMesh()->SnapshotPose(RagdollPose);
  // 传递给 AnimBP
  Cast<UMyAnimBP>(GetMesh()->GetAnimInstance())->SavedRagdollPose = RagdollPose;

Step 3：关闭物理，回到动画
  GetMesh()->SetSimulatePhysics(false);
  GetMesh()->SetAllBodiesSimulatePhysics(false);
  GetMesh()->PhysicsBlendWeight = 1.0f;  // 还是物理姿势（过渡开始点）

Step 4：恢复胶囊位置
  FVector PelvisLoc = GetMesh()->GetBoneLocation("pelvis");
  FRotator PelvisRot = GetMesh()->GetBoneQuaternion("pelvis").Rotator();
  FVector CapsulePos = PelvisLoc;
  CapsulePos.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
  SetActorLocation(CapsulePos, false, nullptr, ETeleportType::TeleportPhysics);

Step 5：播放起身 Montage（从布娃娃姿势 → 站立）
  PlayAnimMontage(GetUpMontage);
  // AnimBP 中：
  //   Saved Ragdoll Pose（PoseSnapshot 节点）→ 混合到起身动画
  //   PhysicsBlendWeight 在 Montage 播放过程中从 1→0
  
Step 6：恢复胶囊碰撞
  GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  GetCharacterMovement()->SetMovementMode(MOVE_Walking);
```

## 二、AnimBP 中的 PoseSnapshot 节点

```
AnimGraph 节点连线：
  [Pose Snapshot] ──────────┐
                            ├─→ [Blend Poses by bool] → [Output Pose]
  [Normal Locomotion Graph] ─┘
  
  Blend Poses by bool：
    Active Value = bIsRagdolling
    True Pose = Pose Snapshot（布娃娃姿势）
    False Pose = 正常动画
    Blend Time = 起身动画时长（如 1.5s）

PoseSnapshot 节点属性：
  Snapshot Name：与 SnapshotPose 时传入的名字一致
  或直接通过蓝图变量引用 FPoseSnapshot 结构体
```

## 三、延伸阅读

- 📄 [5.7 受击反应](./07-hit-reaction.md)
- 📄 [5.8 死亡动画过渡](./08-death-animation.md)
