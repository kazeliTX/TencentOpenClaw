# 1.2 骨骼资产（Skeleton Asset）详解

> **难度**：⭐⭐☆☆☆ | **阅读时间**：约 25 分钟

---

## 一、什么是 Skeleton 资产？

`USkeleton`（骨骼资产）是 UE 动画系统的**基础锚点**。它本质上是一份**骨骼名称层级表**，定义了骨骼的父子关系，但**不包含**具体的网格体形状或蒙皮数据。

> 📌 **关键理解**：Skeleton 是"骨架的规范定义"，而不是"具体的骨头形状"。
> 多个不同外形的 Skeletal Mesh 可以共享同一个 Skeleton，从而共享所有动画资产。

---

## 二、Skeleton 的内部结构

```
USkeleton
│
├── BoneTree（骨骼树）
│   ├── [0] root               parent: -1（根骨骼，无父级）
│   ├── [1] pelvis             parent: 0
│   ├── [2] spine_01           parent: 1
│   ├── [3] spine_02           parent: 2
│   ├── [4] spine_03           parent: 3
│   ├── [5] neck_01            parent: 4
│   ├── [6] head               parent: 5
│   ├── [7] clavicle_l         parent: 4
│   ├── [8] upperarm_l         parent: 7
│   ├── [9] lowerarm_l         parent: 8
│   ├── [10] hand_l            parent: 9
│   ├── [11] thigh_l           parent: 1
│   ├── [12] calf_l            parent: 11
│   ├── [13] foot_l            parent: 12
│   ├── [14] ball_l            parent: 13
│   └── ...（右侧对称，IK 骨骼等）
│
├── ReferenceSkeleton（参考骨骼，含 Ref Pose）
│   └── 每根骨骼在 A-Pose/T-Pose 下的 Local Transform
│
├── Sockets（插槽）
│   ├── weapon_r    → 绑定在 hand_r，偏移 (5, 0, 0)
│   ├── muzzle      → 绑定在 hand_r，偏移 (30, 0, 2)
│   └── head_top    → 绑定在 head
│
├── SmartNameContainer（曲线名称注册表）
│   ├── "MorphTarget_Smile"
│   ├── "MorphTarget_Blink_L"
│   └── ...
│
└── AnimRetargetSources（重定向源）
    └── 用于 IK Retargeter 的参考 Pose 数据
```

---

## 三、骨骼命名规范

UE5 内置的 Mannequin 骨骼采用以下命名规范，建议自定义角色也遵循：

| 骨骼区域 | 命名格式 | 示例 |
|---------|---------|------|
| 主干 | `spine_XX` | `spine_01`, `spine_02` |
| 头颈 | `neck_XX`, `head` | `neck_01`, `head` |
| 手臂 | `clavicle_X`, `upperarm_X`, `lowerarm_X`, `hand_X` | `upperarm_l` |
| 腿部 | `thigh_X`, `calf_X`, `foot_X`, `ball_X` | `thigh_r` |
| IK 骨骼 | `ik_foot_X`, `ik_hand_X` | `ik_foot_l` |
| 武器 | `weapon_X` 或 Socket | `weapon_r` |
| 根骨骼 | `root` 或 `Root` | `root` |

> ⚠️ **注意**：左/右使用 `_l` / `_r` 后缀（小写），不要用 `_L` / `_R` 大写，UE 内部对大小写不敏感但建议统一。

---

## 四、Skeleton 编辑器功能

在 UE 编辑器中双击 Skeleton 资产打开 **Skeleton Editor**：

```
Skeleton Editor 界面：
┌──────────────────────────────────────────────────────┐
│  工具栏                                               │
│  [Skeleton] [Mesh] [Animation] [AnimBP] [Physics]    │
├────────────┬─────────────────────────┬───────────────┤
│ 骨骼树      │      视口预览            │  Details面板  │
│            │                         │               │
│ ▶ root     │   [角色 T-Pose 预览]     │  选中骨骼的    │
│   ▶ pelvis │                         │  属性设置      │
│     ▶ ...  │                         │               │
├────────────┴─────────────────────────┴───────────────┤
│  Asset Browser（动画资产浏览器）                        │
│  [AnimSeq_Idle] [AnimSeq_Walk] [AnimSeq_Run] ...      │
└──────────────────────────────────────────────────────┘
```

### 4.1 主要功能

| 功能 | 操作 | 说明 |
|------|------|------|
| 查看骨骼层级 | 骨骼树面板 | 可折叠展开，右键可选择高亮 |
| 添加 Socket | 骨骼右键 → Add Socket | 创建挂载点 |
| 编辑 Socket | 选中 Socket → Details | 调整位置/旋转偏移 |
| Retargeting 设置 | 骨骼右键 → Recursively Set Translation Retargeting | 配置动画重定向 |
| 查看曲线名称 | Window → Curve Names | 管理 AnimCurve 注册表 |
| 预览姿势 | Asset Browser → 双击动画 | 在视口中预览动画播放 |

---

## 五、多 Mesh 共享 Skeleton

这是 UE 动画系统最强大的特性之一：

```
USkeleton_Warrior（共享骨骼资产）
│
├── SK_Warrior_Male.uasset      体型不同，共享骨骼
├── SK_Warrior_Female.uasset    体型不同，共享骨骼
├── SK_Warrior_Armored.uasset   穿盔甲版本，共享骨骼
└── SK_Warrior_NPC_A.uasset     NPC 变体，共享骨骼
         │
         ▼
所有上述 Mesh 可直接使用同一套动画：
• AS_Idle, AS_Walk, AS_Run, AS_Attack_01 ...
无需重复制作，节省大量工作量！
```

**实现条件**：
1. 骨骼名称完全匹配（区分大小写请保持一致）
2. 骨骼层级相同（父子关系一致）
3. 骨骼数量可以不同（目标 Mesh 可以有更少的骨骼）

---

## 六、Socket 详解

Socket（插槽）是绑定到骨骼的**具名锚点**，用于武器、特效、配件的精确挂载：

### 6.1 Socket 类型

| Socket 类型 | 说明 | 使用场景 |
|------------|------|--------|
| **Skeleton Socket** | 定义在 Skeleton 资产上，所有共享该骨骼的 Mesh 都能使用 | 武器、通用特效 |
| **Mesh Socket** | 定义在特定 Skeletal Mesh 上，仅该 Mesh 可用 | Mesh 特定的细节挂载 |

### 6.2 C++ 使用 Socket

```cpp
// 获取 Socket 的世界变换
FTransform SocketTransform = GetMesh()->GetSocketTransform(
    FName("weapon_r"),
    ERelativeTransformSpace::RTS_World  // 世界空间
);

// 将物体 Attach 到 Socket
WeaponActor->AttachToComponent(
    GetMesh(),
    FAttachmentTransformRules(
        EAttachmentRule::SnapToTarget,   // 位置：对齐到 Socket
        EAttachmentRule::SnapToTarget,   // 旋转：对齐到 Socket
        EAttachmentRule::KeepRelative,   // 缩放：保持相对
        true                             // 焊接 Simulated Bodies
    ),
    FName("weapon_r")
);

// 检查 Socket 是否存在
bool bExists = GetMesh()->DoesSocketExist(FName("weapon_r"));
```

---

## 七、动画重定向（Retargeting）基础

重定向允许将为 A 骨骼制作的动画，应用到骨骼比例不同的 B 骨骼上。

### 7.1 骨骼平移重定向模式

在 Skeleton Editor 中，每根骨骼都可以配置 **Translation Retargeting** 模式：

| 模式 | 说明 | 建议骨骼 |
|------|------|--------|
| `Animation` | 直接使用动画中的平移值 | 所有骨骼默认 |
| `Skeleton` | 使用目标骨骼自身的参考平移（忽略动画平移）| 根骨骼、盆骨 |
| `AnimationScaled` | 按骨骼比例缩放动画平移值 | 四肢 |
| `AnimationRelative` | 使用动画中相对参考姿势的平移增量 | 面部骨骼 |

> 📸 **图示说明**：在 Skeleton Editor → 骨骼树 → 右键骨骼 → Bone Translation Retargeting → 可以看到并修改每根骨骼的重定向模式。

---

## 八、常见问题

### Q: 为什么导入后骨骼和原来 DCC 里的不一样？

**原因**：UE 会对骨骼进行**规范化处理**，移除对蒙皮无贡献的辅助骨骼，并可能调整坐标系（Maya Y-Up → UE Z-Up）。

**解决**：在 FBX 导入设置中关闭 "Convert Scene"，或在 Maya 中以 Z-Up 模式导出。

### Q: 可以在运行时给骨骼添加新的骨骼吗？

**不可以**。骨骼层级在资产导入时就固定了，运行时无法修改。如需动态挂点，使用 **Socket** 或 **PhysicsConstraint** 代替。

### Q: 多个 Skeletal Mesh 部件如何共享骨骼？

使用 **Master Pose Component** 特性：

```cpp
// 设置主 Mesh（驱动骨骼）
USkeletalMeshComponent* MasterMesh = GetMesh();  // 主角身体

// 将其他 Mesh 部件（头发、衣物等）设置为跟随主 Mesh 的骨骼
HairMeshComp->SetMasterPoseComponent(MasterMesh);
ArmorMeshComp->SetMasterPoseComponent(MasterMesh);
// 这样所有部件共享主 Mesh 的骨骼计算结果，零额外开销
```

---

## 九、延伸阅读

- 📄 [1.3 骨骼网格体详解](./03-skeletal-mesh.md)
- 📄 [1.5 骨骼空间与坐标系](./05-coordinate-systems.md)
- 🔗 [Epic 官方：骨骼资产文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/skeletons-in-unreal-engine)
- 🔗 [UE5 Animation 基础概念总结（知乎）](https://zhuanlan.zhihu.com/p/671487108)
