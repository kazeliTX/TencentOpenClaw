# 1.3 骨骼网格体（Skeletal Mesh）详解

> **难度**：⭐⭐☆☆☆ | **阅读时间**：约 20 分钟

---

## 一、Skeletal Mesh 的组成

Skeletal Mesh（骨骼网格体）由两部分组成：

```
USkeletalMesh
│
├── 网格体数据（Geometry）
│   ├── 顶点（Vertices）：位置、法线、UV、切线
│   ├── 三角面（Triangles）：顶点索引列表
│   └── 材质分段（Sections）：按材质划分的网格体片段
│
└── 绑定数据（Skinning）
    ├── 对应的 USkeleton（骨骼资产引用）
    ├── 蒙皮权重（每顶点最多影响 8 根骨骼）
    └── Inverse Bind Pose Matrices（逆绑定姿势矩阵）
```

---

## 二、LOD 配置

Skeletal Mesh 支持多级 LOD，根据角色到摄像机的距离自动切换：

| LOD 级别 | 建议距离 | 三角面数 | 骨骼数 | 动画质量 |
|---------|---------|---------|-------|--------|
| LOD0 | 0~300cm | 100% | 全骨骼 | 完整 |
| LOD1 | 300~800cm | 50% | 减少次要骨骼 | 关闭 IK |
| LOD2 | 800~2000cm | 20% | 核心骨骼 | 简化 AnimBP |
| LOD3 | 2000cm+ | 5% | 最少骨骼 | 单一动画 |

```cpp
// 手动设置 LOD 偏移（调试用）
GetMesh()->SetForcedLOD(2);  // 强制使用 LOD2
GetMesh()->SetForcedLOD(0);  // 恢复自动 LOD

// 设置 LOD 距离（屏幕占比阈值）
GetMesh()->SetCachedMaxDrawDistance(2000.0f);
```

---

## 三、材质与 Mesh Section

一个 Skeletal Mesh 可以有多个材质分段（Sections），每个 Section 使用不同材质：

```cpp
// 运行时替换材质
GetMesh()->SetMaterial(0, NewBodyMaterial);    // Section 0：身体
GetMesh()->SetMaterial(1, NewArmorMaterial);   // Section 1：盔甲

// 创建动态材质实例（修改参数）
UMaterialInstanceDynamic* DynMat =
    GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
DynMat->SetScalarParameterValue(FName("Damage"), 0.8f);
DynMat->SetVectorParameterValue(FName("TintColor"), FLinearColor::Red);
```

---

## 四、Morph Target（形变目标）

Morph Target 是顶点级别的形变数据，常用于面部表情：

```cpp
// 设置 Morph Target 权重（0.0 = 无形变，1.0 = 完全形变）
GetMesh()->SetMorphTarget(FName("MorphTarget_Smile"), 0.75f);
GetMesh()->SetMorphTarget(FName("MorphTarget_Blink_L"), 1.0f);

// 获取当前权重
float CurrentWeight = GetMesh()->GetMorphTarget(FName("MorphTarget_Smile"));

// 清空所有 Morph Target
GetMesh()->ClearMorphTargets();
```

---

## 五、Master Pose Component（多部件共享骨骼）

用于将多个 Mesh 部件（头发、衣物、配件）的骨骼驱动统一到主 Mesh：

```cpp
// 在角色 BeginPlay 中设置
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    USkeletalMeshComponent* BodyMesh = GetMesh();  // 主体

    // 头发、衣物等跟随主体骨骼（零额外骨骼计算开销）
    HairMesh->SetMasterPoseComponent(BodyMesh);
    CloakMesh->SetMasterPoseComponent(BodyMesh);
    ArmorMesh->SetMasterPoseComponent(BodyMesh);
}
```

> ⚠️ **注意**：使用 Master Pose Component 的子 Mesh 无法独立播放动画，也无法使用 AnimBP。如需独立动画，改用 `CopyPoseFromMesh`。

---

## 六、延伸阅读

- 📄 [1.2 骨骼资产详解](./02-skeleton-asset.md)
- 🔗 [Working with Skeletal Mesh in UE](https://vrealmatic.com/unreal-engine/skeletal-mesh)
- 🔗 [FBX 导入管线](https://dev.epicgames.com/documentation/en-us/unreal-engine/fbx-skeletal-mesh-pipeline-in-unreal-engine)
