# 2.2 参考姿势与绑定矩阵

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、什么是参考姿势（Reference Pose）

参考姿势（也叫 Bind Pose / Rest Pose）是骨骼的**静止基准状态**，是在 DCC 软件中完成蒙皮时的骨骼姿势。

```
常见参考姿势类型：

T-Pose（T 形姿势）：
  • 双臂水平伸展，双腿并拢
  • 历史悠久，Maya 传统默认
  • 缺点：肩部蒙皮在自然状态下变形较大

    ───O───
       │
      / \

A-Pose（A 形姿势）：
  • 双臂向下 45°，更接近自然站姿
  • UE5 Mannequin 使用 A-Pose
  • 优点：肩部区域蒙皮更自然
  • 推荐用于现代游戏角色

     \O/（45°斜臂）
      │
     / \
```

### 为什么参考姿势很重要？

```
参考姿势 = 蒙皮权重的"零点"

当角色处于参考姿势时：
  顶点位置 = 模型文件中的原始位置（无变形）
  蒙皮矩阵 = 单位矩阵（不做任何变换）

当骨骼偏离参考姿势时：
  顶点被各骨骼矩阵加权变换
  变形量 = 偏离参考姿势的程度

结论：参考姿势越接近动画的"平均姿势"，蒙皮效果越好
```

---

## 二、绑定姿势矩阵（Bind Pose Matrix）

### 2.1 定义

```
对于每根骨骼 i，定义：

BindPoseMatrix_i  = 骨骼 i 在参考姿势下的世界变换矩阵
InvBindPose_i     = BindPoseMatrix_i 的逆矩阵

InvBindPose 在 FBX 导入时由 UE 自动计算并存储
存储位置：USkeletalMesh 内部（对应 FSkelMeshRenderSection 中的骨骼矩阵数组）
```

### 2.2 蒙皮矩阵的计算

```
每帧，GPU 蒙皮需要计算每根骨骼的"蒙皮矩阵"：

SkinMatrix_i = InvBindPose_i × CurrentBoneWorld_i

含义：
  1. InvBindPose_i：将顶点从模型空间变换到骨骼 i 的参考局部空间
  2. CurrentBoneWorld_i：将顶点从骨骼局部空间变换回世界空间（但是当前帧的姿势）

最终顶点位置（多骨骼加权）：
  FinalPos = Σ(weight[i] × SkinMatrix[i] × OriginalVertexPos)
           = Σ(weight[i] × InvBindPose[i] × CurrentBoneWorld[i] × V)
```

### 2.3 UE 中的数据流

```
[USkeleton::RawRefBonePose]    参考姿势（Local Space）
          │
          ▼ FK 计算（参考姿势下）
[BindPoseMatrix]               参考姿势（World Space）
          │
          ▼ 求逆
[InvBindPoseMatrix]            存储在 USkeletalMesh
          │
          │（每帧）
          ▼
          InvBind × CurrentWorld → SkinMatrix → GPU
```

---

## 三、重定向参考姿势（Retarget Base Pose）

UE5 IK Retargeter 中有一个容易混淆的概念：

```
普通参考姿势（Reference Pose）：
  骨骼资产中定义，用于蒙皮计算，不可运行时修改

重定向参考姿势（Retarget Base Pose）：
  专为动画重定向设计
  可在 IK Rig 编辑器中调整
  作用：让两个骨骼比例不同的角色能正确对齐

典型用例：
  Source 角色（UE5 Mannequin，A-Pose）→ 重定向 → Target 角色（Mixamo，T-Pose）
  需要在 IK Retargeter 中调整 Retarget Base Pose 让两者对齐
  否则重定向后角色姿势会有明显偏差
```

---

## 四、查询参考姿势数据（C++）

```cpp
// 获取骨骼的参考姿势（Local Space）
FTransform GetRefPoseLocalTransform(
    USkeletalMeshComponent* Mesh,
    FName BoneName)
{
    if (!Mesh || !Mesh->GetSkeletalMeshAsset()) return FTransform::Identity;

    const FReferenceSkeleton& RefSkel =
        Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();

    int32 BoneIdx = RefSkel.FindBoneIndex(BoneName);
    if (BoneIdx == INDEX_NONE) return FTransform::Identity;

    // RawRefBonePose 存储的是 Local Space 参考变换
    return RefSkel.GetRefBonePose()[BoneIdx];
}

// 计算骨骼在参考姿势下的 Component Space 变换（需要 FK）
FTransform GetRefPoseComponentTransform(
    USkeletalMeshComponent* Mesh,
    FName BoneName)
{
    if (!Mesh || !Mesh->GetSkeletalMeshAsset()) return FTransform::Identity;

    const FReferenceSkeleton& RefSkel =
        Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();

    int32 TargetIdx = RefSkel.FindBoneIndex(BoneName);
    if (TargetIdx == INDEX_NONE) return FTransform::Identity;

    const TArray<FTransform>& RefPoses = RefSkel.GetRefBonePose();

    // 从根骨骼开始，沿路径累乘到目标骨骼
    TArray<int32> BonePath;
    int32 Current = TargetIdx;
    while (Current != INDEX_NONE)
    {
        BonePath.Insert(Current, 0);
        Current = RefSkel.GetParentIndex(Current);
    }

    FTransform ComponentTransform = FTransform::Identity;
    for (int32 Idx : BonePath)
    {
        ComponentTransform = RefPoses[Idx] * ComponentTransform;
    }
    return ComponentTransform;
}

// 检查当前姿势与参考姿势的偏差（调试用）
float GetBoneDeviationFromRefPose(
    USkeletalMeshComponent* Mesh,
    FName BoneName)
{
    FTransform RefComp    = GetRefPoseComponentTransform(Mesh, BoneName);
    FTransform CurrentComp = Mesh->GetBoneTransform(Mesh->GetBoneIndex(BoneName));

    // 计算旋转偏差角度（度）
    FQuat RefRot     = RefComp.GetRotation();
    FQuat CurrentRot = CurrentComp.GetRotation();
    float Angle = FMath::RadiansToDegrees(RefRot.AngularDistance(CurrentRot));
    return Angle;
}
```

---

## 五、参考姿势常见问题

### Q: 导入后参考姿势和 DCC 里的不一致？

```
原因：UE 导入时会做以下处理：
  1. 坐标系转换（Y-Up → Z-Up）
  2. 单位转换（Maya cm/inch → UE cm）
  3. 骨骼方向规范化

解决：
  • 在 Maya/Blender 中以 Z-Up 导出
  • 确保场景单位为 cm
  • 导入时 "Import Uniform Scale" 设为 1.0
```

### Q: 不同 DCC 软件的参考姿势可以混用吗？

```
不推荐。每个 DCC 软件和角色的参考姿势可能不同（T/A-Pose 角度不同）。
如需重定向，使用 IK Retargeter 的 Retarget Base Pose 功能对齐。
```

---

## 六、延伸阅读

- 📄 [2.3 蒙皮算法：LBS vs DQS](./03-skinning-algorithms.md)
- 🔗 [Epic 官方：Skeleton 资产](https://dev.epicgames.com/documentation/en-us/unreal-engine/skeletons-in-unreal-engine)
- 🔗 [IK Retargeter 重定向文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/ik-rig-animation-retargeting-in-unreal-engine)
