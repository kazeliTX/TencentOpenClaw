# 2.1 骨骼层级深度解析

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、FReferenceSkeleton 数据结构

`FReferenceSkeleton` 是 UE 内部存储骨骼层级的核心结构，存在于 `USkeleton` 和 `USkeletalMesh` 两个资产中（各自维护一份，通常相同）。

```cpp
// Engine/Source/Runtime/Engine/Public/ReferenceSkeleton.h（简化版）

struct FMeshBoneInfo
{
    FName   Name;         // 骨骼名称（如 "head"）
    int32   ParentIndex;  // 父骨骼索引，根骨骼为 -1（INDEX_NONE）
    // 注：FString ExportName 用于 FBX 导出，通常与 Name 相同
};

struct FReferenceSkeleton
{
    // 骨骼元信息数组（名称 + 父索引）
    TArray<FMeshBoneInfo>  RawRefBoneInfo;

    // 参考姿势下每根骨骼的 Local Space Transform（A-Pose / T-Pose）
    TArray<FTransform>     RawRefBonePose;

    // 最终合并后的骨骼信息（包含来自 Skeleton 和 Mesh 的所有骨骼）
    TArray<FMeshBoneInfo>  FinalRefBoneInfo;
    TArray<FTransform>     FinalRefBonePose;

    // 名称 → 索引映射（加速查找）
    TMap<FName, int32>     RawNameToIndexMap;
    TMap<FName, int32>     FinalNameToIndexMap;

    // ... 其他方法
};
```

### 骨骼索引的重要性

```
骨骼索引（Bone Index）是动画系统的基础单位：
• 动画数据按索引存储（而非按名称），查询极快 O(1)
• 所有矩阵数组都按索引对齐
• 同一 Skeleton 的所有动画资产共享相同的索引映射

关键：一旦骨骼从 Skeleton 中删除或顺序改变，
      所有依赖该 Skeleton 的动画都需要重新导入！
      → 这是为什么在项目中期不要轻易改变骨骼层级
```

---

## 二、骨骼数组的内存布局

UE 骨骼数组采用**深度优先遍历顺序**（Depth-First Order）排列，这一设计对性能至关重要：

```
骨骼树结构：
        root [0]
         │
       pelvis [1]
      /        \
  spine_01[2]  thigh_l[6]
     │               │
  spine_02[3]    calf_l[7]
     │               │
  spine_03[4]   foot_l[8]
     │
  neck[5]

内存中的骨骼数组（深度优先）：
  Index 0: root        parent=-1
  Index 1: pelvis      parent=0
  Index 2: spine_01    parent=1
  Index 3: spine_02    parent=2
  Index 4: spine_03    parent=3
  Index 5: neck        parent=4
  Index 6: thigh_l     parent=1
  Index 7: calf_l      parent=6
  Index 8: foot_l      parent=7

性能优势：
  FK 正向计算时，父骨骼 Index < 子骨骼 Index（永远成立！）
  → 可以用单次线性扫描完成所有骨骼的世界变换计算
  → 无需递归，对 CPU 缓存极为友好
```

---

## 三、FillComponentSpaceTransforms（FK 核心计算）

这是 UE 动画系统每帧最核心的计算函数之一：

```cpp
// 简化的 FK 计算实现（概念版）
void FillComponentSpaceTransforms(
    const TArray<FTransform>& LocalPose,        // 输入：Local Space 姿势
    TArray<FTransform>& OutComponentPose,        // 输出：Component Space 姿势
    const FReferenceSkeleton& RefSkel)
{
    int32 NumBones = LocalPose.Num();
    OutComponentPose.SetNum(NumBones);

    // 根骨骼直接使用 Local Transform（无父级）
    OutComponentPose[0] = LocalPose[0];

    // 因为深度优先排列，父骨骼索引 < 子骨骼索引
    // 可以保证当处理骨骼 i 时，其父骨骼已经计算完毕
    for (int32 i = 1; i < NumBones; ++i)
    {
        int32 ParentIdx = RefSkel.GetParentIndex(i);
        
        // Component Space = Local × ParentComponentSpace
        OutComponentPose[i] = LocalPose[i] * OutComponentPose[ParentIdx];
        
        // 规范化四元数，防止浮点误差累积
        OutComponentPose[i].NormalizeRotation();
    }
}
// 时间复杂度：O(n)，n = 骨骼数量
// 空间局部性极好（线性访问），对 CPU 缓存非常友好
```

---

## 四、骨骼变换矩阵的完整计算链

```
一根骨骼的最终世界变换，经过以下计算链：

[AnimSequence Key Data]
        │  关键帧插值（Lerp/Slerp）
        ▼
[Local Space Pose]         每根骨骼相对于父骨骼的变换
        │  FK 链式乘法（FillComponentSpaceTransforms）
        ▼
[Component Space Pose]     每根骨骼相对于 SkeletalMeshComponent 的变换
        │  × Component World Transform
        ▼
[World Space Bone Transform]  骨骼在游戏世界的绝对变换
        │  × Inverse Bind Pose Matrix（"蒙皮矩阵"）
        ▼
[Skinning Matrix]          最终传给 GPU 的矩阵（用于顶点蒙皮）
```

### 逆绑定姿势矩阵（Inverse Bind Pose Matrix）

```
绑定姿势（Bind Pose / Reference Pose）：
  角色在被"绑定"（蒙皮权重计算）时的静止姿势（通常是 T-Pose 或 A-Pose）

逆绑定姿势矩阵的作用：
  顶点在模型空间下是相对于 Bind Pose 定义的
  当骨骼运动时，需要先把顶点"变回"到骨骼的局部空间（乘以逆绑定矩阵）
  再乘以骨骼当前的变换，得到变形后的位置

公式：
  FinalVertexPos = Σ(weight_i × (CurrentBoneMatrix_i × InvBindPose_i) × OriginalVertexPos)

简化理解：
  逆绑定矩阵 = "骨骼绑定时的位置的逆"
  乘以逆绑定矩阵后，顶点从模型空间变换到骨骼局部空间
  再乘以当前骨骼变换，输出最终位置
```

---

## 五、骨骼数量对性能的影响

```
骨骼数量 vs 性能开销（经验参考值）：

< 50 骨骼：  极轻量，不影响性能
50-100：      适中，主流手机角色标准
100-200：     较重，PC/主机游戏常见
200-300：     重，需要 LOD 骨骼裁减
> 300：       很重，需要极致优化（URO、禁用次要骨骼等）

每帧 FK 计算成本（大约参考）：
  100 骨骼角色 @ 100 个并发：~ 2-5ms（Worker Thread）
  
主要优化手段：
  1. LOD 削减骨骼（远处 Mesh 用更少骨骼）
  2. URO（Update Rate Optimization）：远处角色不每帧更新动画
  3. 关闭不可见角色的动画更新
  4. Master Pose Component：多部件共享骨骼计算
```

---

## 六、C++ 遍历骨骼层级

```cpp
// 完整的骨骼层级遍历示例
void IterateBoneHierarchy(USkeletalMeshComponent* Mesh)
{
    if (!Mesh || !Mesh->GetSkeletalMeshAsset()) return;

    const FReferenceSkeleton& RefSkel =
        Mesh->GetSkeletalMeshAsset()->GetRefSkeleton();
    int32 NumBones = RefSkel.GetNum();

    UE_LOG(LogTemp, Log, TEXT("=== Bone Hierarchy (%d bones) ==="), NumBones);

    for (int32 i = 0; i < NumBones; ++i)
    {
        FName BoneName   = RefSkel.GetBoneName(i);
        int32 ParentIdx  = RefSkel.GetParentIndex(i);
        FName ParentName = (ParentIdx != INDEX_NONE)
            ? RefSkel.GetBoneName(ParentIdx)
            : FName("(root)");

        // 计算缩进深度
        int32 Depth = 0;
        int32 Tmp   = ParentIdx;
        while (Tmp != INDEX_NONE)
        {
            ++Depth;
            Tmp = RefSkel.GetParentIndex(Tmp);
        }

        FString Indent = FString::ChrN(Depth * 2, ' ');
        UE_LOG(LogTemp, Log, TEXT("%s[%d] %s  (parent: %s [%d])"),
            *Indent, i, *BoneName.ToString(),
            *ParentName.ToString(), ParentIdx);
    }
}

// 查找两根骨骼的最近公共祖先
int32 FindCommonAncestor(
    const FReferenceSkeleton& RefSkel,
    int32 BoneA,
    int32 BoneB)
{
    // 收集 A 的所有祖先
    TSet<int32> AncestorsA;
    int32 Current = BoneA;
    while (Current != INDEX_NONE)
    {
        AncestorsA.Add(Current);
        Current = RefSkel.GetParentIndex(Current);
    }

    // 从 B 向上找，第一个出现在 AncestorsA 中的就是公共祖先
    Current = BoneB;
    while (Current != INDEX_NONE)
    {
        if (AncestorsA.Contains(Current))
            return Current;
        Current = RefSkel.GetParentIndex(Current);
    }
    return INDEX_NONE;
}
```

---

## 七、延伸阅读

- 📄 [2.2 参考姿势与绑定矩阵](./02-reference-pose.md)
- 📄 [2.3 蒙皮算法：LBS vs DQS](./03-skinning-algorithms.md)
- 🔗 [UE 源码：ReferenceSkeleton.h](https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Engine/Public/ReferenceSkeleton.h)
- 🔗 [剖析 UE 动画系统核心脉络](https://segmentfault.com/a/1190000044591404)
