# 10.8 IK Retargeter 动画重定向

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、动画重定向需求

```
场景：
  商店购买了一套动画（Mixamo/Motion Capture）
  但这套动画的骨骼与你的角色骨骼不同（命名/比例/数量）
  需要将动画"迁移"到你的角色骨骼上

传统方案：手动在 DCC 中重定向（耗时，有误差）
UE5 IK Retargeter：在引擎内自动重定向（快速，质量好）
```

---

## 二、重定向工作流

```
步骤 1：为源骨骼创建 IK Rig
  内容浏览器 → 右键 SourceMesh → Create → IK Rig
  命名：IKRig_Source
  配置：Retarget Chains（骨骼链映射）

步骤 2：为目标骨骼创建 IK Rig
  同样步骤，命名：IKRig_Target
  配置：与源 IK Rig 对应的骨骼链

步骤 3：创建 IK Retargeter
  内容浏览器 → 右键 → Animation → IK Retargeter
  命名：RTG_SourceToTarget
  
  Source IK Rig: IKRig_Source
  Target IK Rig: IKRig_Target
  
  Source Preview Mesh: SourceCharacter.fbx
  Target Preview Mesh: TargetCharacter.fbx

步骤 4：对齐 T-Pose（重要！）
  将两个角色都摆成相同的 T-Pose 或 A-Pose
  骨骼方向一致 → 重定向结果最准确
  
  Edit Retarget Pose → 调整到匹配 T-Pose

步骤 5：链映射
  Chain Mapping 面板：
    Source Chain    Target Chain
    Spine      →    Spine
    LeftArm    →    LeftArm
    RightLeg   →    RightLeg
    Head       →    Head
  
  UE 通常自动匹配，手动检查并修正

步骤 6：导出重定向动画
  选择源动画 → Export Retargeted Animations
  → 生成目标骨骼的新动画资产
```

---

## 三、运行时重定向

```
UE5.2+ 支持运行时 IK Retargeting（不预先导出）：

AnimGraph 节点：Retarget Pose From Mesh
  Source Mesh Component: 源角色的 SkeletalMeshComponent
  IK Retargeter Asset: RTG_SourceToTarget
  → 实时将源角色的姿势重定向到目标角色

应用场景：
  多人游戏中不同体型角色共享同一套动画
  过场动画中不同 NPC 使用同一动画师制作的动画
  MetaHuman 系统（见第十一章）
```

---

## 四、延伸阅读

- 📄 [代码示例：IK 重定向辅助](./code/03_ik_retarget_helper.cpp)
- 🔗 [IK Retargeter 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-animation-retargeting-in-unreal-engine)
