# 5.8 IK Retargeter 动画重定向

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、动画重定向的意义

```
问题：
  购买了 Mixamo 动画包，但项目角色是 UE Mannequin 骨骼
  两者骨骼比例、命名、层级都不同
  
IK Retargeter 解决：
  自动将 A 骨骼的动画转换到 B 骨骼
  支持不同骨骼比例（高矮胖瘦角色）
  支持运行时实时重定向（不需要预先导出）
```

---

## 二、配置 IK Retargeter

```
步骤：

1. 为 Source（源）骨骼创建 IK Rig：
   IKR_Source（如 Mixamo IK Rig）
   配置 Retarget Chains（见 5.6）

2. 为 Target（目标）骨骼创建 IK Rig：
   IKR_Target（如 UE Mannequin IK Rig）
   配置对应的 Retarget Chains

3. 创建 IK Retargeter：
   内容浏览器 → 右键 → Animation → IK Retargeter
   Source IK Rig: IKR_Source
   Target IK Rig: IKR_Target

4. 在 IK Retargeter 编辑器中：
   链映射（Chain Mapping）：
   Target Chain    Source Chain
   ─────────────────────────────
   Root        ←→ Root
   Spine       ←→ Spine
   LeftArm     ←→ LeftArm
   RightArm    ←→ RightArm
   LeftLeg     ←→ LeftLeg
   RightLeg    ←→ RightLeg
   Head        ←→ Head
```

---

## 三、重定向参考姿势对齐

```
关键步骤：对齐 Source 和 Target 的参考姿势

问题：Mixamo 是 T-Pose，UE Mannequin 是 A-Pose
     直接重定向会导致手臂角度错误

解决：
  IK Retargeter 编辑器 → Retarget Pose 标签页
  Edit Mode: Source / Target
  
  调整 Target 的 Retarget Pose（Mannequin）：
  将 Mannequin 的手臂从 A-Pose 调整到 T-Pose（临时）
  或将 Mixamo 从 T-Pose 调整到 A-Pose
  → 让两者参考姿势对齐
  
  注意：这个调整只影响重定向，不影响正常动画播放
```

---

## 四、运行时重定向

```cpp
// 运行时使用 IK Retargeter（UE 5.1+）
// 注意：运行时重定向比离线导出慢，适合少量角色

#include "Retargeter/IKRetargeter.h"
#include "Retargeter/IKRetargetProcessor.h"

// 在角色初始化时创建重定向处理器
void AMyCharacter::SetupRetargeting()
{
    UIKRetargeter* RetargeterAsset = LoadObject<UIKRetargeter>(
        nullptr, TEXT("/Game/Retargeters/RTG_MixamoToMannequin"));

    if (RetargeterAsset)
    {
        RetargetProcessor = UIKRetargetProcessor::StaticClass()->
            GetDefaultObject<UIKRetargetProcessor>();
        RetargetProcessor->Initialize(
            SourceMesh->GetSkeletalMeshAsset(),
            TargetMesh->GetSkeletalMeshAsset(),
            RetargeterAsset,
            false); // bSuppressWarnings
    }
}

// 每帧更新
void AMyCharacter::UpdateRetargeting(float DeltaTime)
{
    if (!RetargetProcessor) return;
    
    // 获取源骨骼姿势（Component Space）
    TArray<FTransform> SourcePose;
    SourceMesh->GetEditableComponentSpaceTransforms().GenerateValueArray(SourcePose);

    // 重定向到目标骨骼
    TArray<FTransform> TargetPose;
    RetargetProcessor->RunRetargeter(SourcePose, TargetPose);

    // 应用到目标 Mesh
    // （通常通过 AnimBP 节点而非直接设置）
}
```

---

## 五、批量重定向（编辑器工具）

```
批量将 Mixamo 动画重定向到 Mannequin：

1. 内容浏览器 → 选中所有 Mixamo 动画序列
2. 右键 → Retarget Animation Assets → Duplicate And Retarget
3. 选择 IK Retargeter
4. 目标目录（存放重定向后的动画）
5. 点击 Retarget

生成的动画可以直接在 UE Mannequin 上使用
无需修改动画，完全复用 Mixamo 内容库
```

---

## 六、延伸阅读

- 🔗 [IK Retargeter 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-animation-retargeting-in-unreal-engine)
- 🔗 [Mixamo 到 UE5 重定向教程（知乎）](https://zhuanlan.zhihu.com/p/593031965)
