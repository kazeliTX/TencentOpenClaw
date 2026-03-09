# 4.7 Pose Snapshot 与 Pose Asset

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 20 分钟

---

## 一、Pose Snapshot

Pose Snapshot 允许在运行时**捕获当前骨骼姿势**并在之后使用：

```
典型场景：布娃娃到动画的平滑过渡

问题：角色从布娃娃状态（随机物理姿势）恢复到动画时，
      动画状态机从参考姿势开始，会有明显跳变

解决：
  1. 布娃娃结束时：捕获当前物理姿势 → 存入 Snapshot
  2. AnimBP 使用 "Snapshot Pose" 节点从 Snapshot 播放
  3. 同时播放站起来的动画
  4. 混合：Snapshot Pose → Stand Up Animation（Alpha 从 1.0 → 0.0）
  5. 效果：角色从当前躺倒姿势平滑过渡到站起动画
```

### C++ 捕获 Pose Snapshot

```cpp
// 捕获当前姿势
void AMyCharacter::CapturePoseSnapshot()
{
    UAnimInstance* Anim = GetMesh()->GetAnimInstance();
    if (!Anim) return;

    // 捕获到 SavedPoseSnapshot 变量（FPoseSnapshot）
    Anim->SnapshotPose(SavedPoseSnapshot);
    UE_LOG(LogTemp, Log, TEXT("Pose snapshot captured"));
}

// AnimInstance.h 中声明：
UPROPERTY(BlueprintReadWrite, Category="Snapshot")
FPoseSnapshot SavedPoseSnapshot;
```

### AnimGraph 配置

```
节点：Snapshot Pose
  Snapshot: AnimInstance.SavedPoseSnapshot（变量引用）
  
配合使用：
  [Snapshot Pose]────────┐
                          [Blend]──→ Output
  [Stand Up Anim]────────┘  Alpha: 从1.0→0.0（随时间消散）
```

---

## 二、Pose Asset

Pose Asset 是一组命名姿势的集合，常用于面部动画和 IK 辅助姿势：

```
Pose Asset 的内容：
  一个 Pose Asset 可以包含多个姿势，每个姿势有名称和权重

创建方式：
  1. 录制一系列骨骼姿势（或从动画序列提取帧）
  2. 内容浏览器 → 右键 → Animation → Pose Asset

典型用途：
  面部表情系统：
    口型（Phoneme）：A, E, I, O, U, M, S...
    情绪：Happy, Sad, Angry, Surprised
    → 通过 Pose Driver 节点根据骨骼位置自动混合口型

  IK 辅助姿势：
    手部握枪姿势
    手部抓梯子姿势
    → 通过 Pose By Name 节点按名称激活
```

### 使用 Pose By Name 节点

```
在 AnimGraph 中：
  [任意基础姿势]
        │
  [Apply Additive]
    Base Pose: ← 基础姿势
    Additive:  ← [Pose By Name]
                    Pose Asset: PA_HandGrip（Pose Asset 资产）
                    Pose Name: "Hold_Rifle"（姿势名称）
                    Curve Alpha: 1.0
```

---

## 三、Pose Driver（程序化面部动画）

```
Pose Driver 根据骨骼的运动自动激活 Pose Asset 中的姿势：

配置：
  Source Bones: jaw（下颌骨骼）
  驱动方向：根据 jaw 的旋转量激活口型姿势
  
  自动映射：
    jaw.Pitch = 0°   → 嘴闭合（姿势权重 0.0）
    jaw.Pitch = 15°  → 嘴微开（姿势权重 0.5）
    jaw.Pitch = 30°  → 嘴大开（姿势权重 1.0）
    
用途：
  程序化口型同步（无需逐帧手动设置口型）
  眼部注视自动眨眼（根据眼球方向激活眨眼姿势）
```

---

## 四、延伸阅读

- 📄 [代码示例：Pose Snapshot](./code/03_pose_snapshot.cpp)
- 🔗 [Pose Snapshot 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/pose-snapshot-in-unreal-engine)
- 🔗 [Pose Asset 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/pose-assets-in-unreal-engine)
