# 4.9 Motion Warping 根运动对齐

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、Root Motion 与 Motion Warping 概述

### Root Motion（根运动）

```
Root Motion 让动画中的位移数据驱动角色移动（而非代码）：

普通动画：角色位移 = CharacterMovement 计算
Root Motion：角色位移 = 动画根骨骼的位移数据

优点：动作动画（攻击、翻滚）的位移与动画视觉完全匹配
缺点：位移量固定，无法适应不同距离的目标

启用 Root Motion：
  AnimSequence → Details → Enable Root Motion: ✅
  Root Motion Root Lock: Ref Pose（推荐）
```

### Motion Warping（运动扭曲）

```
Motion Warping = 动态缩放 Root Motion，使其精确对准目标：

场景：角色翻墙动画，Root Motion 假设墙在 200cm 处
     但实际墙可能在 150~250cm 处
     
Motion Warping 解决：
  1. 标记动画中的"关键时刻"（如双手扶墙的帧）
  2. 运行时指定目标点（墙的实际位置）
  3. Motion Warping 自动缩放动画中的位移，使角色在关键帧时精确到达目标
```

---

## 二、配置 Motion Warping

### 2.1 添加 Motion Warping 组件

```cpp
// 在角色类中添加
UPROPERTY(VisibleAnywhere, Category = "Components")
UMotionWarpingComponent* MotionWarpingComp;

// BeginPlay 中初始化
MotionWarpingComp = FindComponentByClass<UMotionWarpingComponent>();
```

### 2.2 在动画中添加 Warping Notify

```
打开动画序列（如 AM_VaultOver_Wall）：
  Notifies 轨道 → 右键 → Add Notify State → Motion Warping
  
配置 Motion Warping Notify State：
  Warp Target Name: VaultTarget（自定义目标名）
  Warp Point Anim Provider: Default
  Warp Translation: ✅（扭曲位移）
  Warp Rotation:    ✅（扭曲旋转，让角色朝向目标）
  Rotation Type:    Facing（朝向目标）
  
  设置区间：从起跳帧到落地帧
```

### 2.3 运行时设置目标

```cpp
// 在角色决定翻墙时，设置 Warping 目标
void AMyCharacter::StartVaultOver(FVector WallTopPosition, FRotator WallNormal)
{
    if (!MotionWarpingComp) return;

    // 添加/更新 Warp Target
    MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
        FName("VaultTarget"),       // 目标名（与 Notify 中一致）
        WallTopPosition,            // 目标位置（墙顶）
        WallNormal.Quaternion()     // 目标朝向（朝向墙的法线方向）
    );

    // 播放翻墙 Montage（内含 Motion Warping Notify）
    PlayAnimMontage(VaultMontage);
}

// 翻墙结束后清理目标（可选）
void AMyCharacter::OnVaultFinished()
{
    MotionWarpingComp->RemoveWarpTarget(FName("VaultTarget"));
}
```

---

## 三、多目标 Motion Warping（攀爬系统）

```cpp
// 攀爬墙壁示例（需要多个 Warp 点）
void AMyCharacter::StartClimb(
    FVector FootPosition,    // 脚踩的位置
    FVector HandPosition,    // 手抓的位置
    FRotator FaceDirection)  // 面对方向
{
    // 设置脚部目标
    MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
        FName("ClimbFoot"),
        FootPosition,
        FaceDirection.Quaternion()
    );

    // 设置手部目标
    MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
        FName("ClimbHand"),
        HandPosition,
        FaceDirection.Quaternion()
    );

    // 播放攀爬 Montage
    PlayAnimMontage(ClimbMontage);
}
```

---

## 四、Motion Warping vs 传统 IK 的区别

```
Motion Warping：
  • 修改的是动画播放位置（Root Motion 级别）
  • 让角色整体移动到对应目标
  • 适合：翻墙、跨越、攀爬等大幅度移动
  • 不改变骨骼相对关系

IK（如 Two-Bone IK）：
  • 修改的是骨骼末端位置
  • 局部调整（如脚踩在不平地面）
  • 适合：小幅度局部调整
  • 改变骨骼相对关系

最佳实践：组合使用
  Motion Warping：让角色主体移动到正确位置
  IK：细调手脚骨骼精确接触表面
```

---

## 五、延伸阅读

- 📄 [代码示例：Motion Warping](./code/02_motion_warping.cpp)
- 🔗 [Motion Warping 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-warping-in-unreal-engine)
- 🔗 [Root Motion 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/root-motion-in-unreal-engine)
- 🔗 [Locomotion-Based Blending](https://dev.epicgames.com/documentation/en-us/unreal-engine/locomotion-based-blending-in-unreal-engine)
