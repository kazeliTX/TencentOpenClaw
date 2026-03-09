# 1.6 动画管线（Pipeline）全流程

> **难度**：⭐⭐☆☆☆ | **阅读时间**：约 20 分钟

---

## 一、从美术资产到游戏画面的完整路径

```
┌─────────────────────────────────────────────────────────────────────┐
│                    完整动画生产管线                                   │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  【美术阶段 - DCC 软件】                                              │
│  ┌──────────┐   ┌──────────┐   ┌──────────────┐                   │
│  │ 建模     │──▶│ 骨骼绑定  │──▶│  动画制作     │                   │
│  │ Maya/Max │   │ Rigging  │   │  Keyframe/   │                   │
│  │ Blender  │   │ Skinning │   │  Mocap/      │                   │
│  └──────────┘   └──────────┘   │  Procedural  │                   │
│                                └──────┬───────┘                   │
│                                       │ 导出 FBX/Alembic           │
│  ┌────────────────────────────────────▼────────────────────────┐   │
│  │                  【UE 导入阶段】                               │   │
│  │                                                              │   │
│  │  FBX Import Wizard                                           │   │
│  │  ├── 生成 USkeleton（首次导入自动创建）                        │   │
│  │  ├── 生成 USkeletalMesh（含蒙皮权重）                         │   │
│  │  ├── 生成 UAnimSequence（关键帧数据）                         │   │
│  │  └── 生成 UMaterial（材质，可选）                             │   │
│  └────────────────────────────────────┬─────────────────────────┘  │
│                                       │                            │
│  ┌────────────────────────────────────▼────────────────────────┐   │
│  │                  【UE 资产整合阶段】                            │   │
│  │                                                              │   │
│  │  • 创建 Animation Blueprint                                  │   │
│  │  • 搭建 State Machine 逻辑                                   │   │
│  │  • 创建 Blend Space（混合多个 AnimSequence）                  │   │
│  │  • 创建 Animation Montage（攻击/受击等）                      │   │
│  │  • 配置 Physics Asset（碰撞体和约束）                         │   │
│  │  • 配置 IK Rig + IK Retargeter（如需重定向）                 │   │
│  └────────────────────────────────────┬─────────────────────────┘  │
│                                       │                            │
│  ┌────────────────────────────────────▼────────────────────────┐   │
│  │                  【运行时评估阶段】每帧执行                     │   │
│  │                                                              │   │
│  │  Game Thread:                                                │   │
│  │  ① Tick → AnimInstance::NativeUpdateAnimation()              │   │
│  │     更新游戏状态变量（速度、方向、血量等）                       │   │
│  │                                                              │   │
│  │  Worker Thread（并行）:                                       │   │
│  │  ② AnimGraph 节点树评估                                       │   │
│  │     State Machine → Blend → IK → Output Pose（Local Space）  │   │
│  │                                                              │   │
│  │  ③ Local Space → Component Space 转换（FillSpaceBases）      │   │
│  │                                                              │   │
│  │  ④ 后处理（布料模拟、物理动画）                                 │   │
│  │                                                              │   │
│  │  Render Thread:                                              │   │
│  │  ⑤ 骨骼矩阵上传 GPU                                          │   │
│  │  ⑥ GPU Skinning → 最终顶点位置                               │   │
│  │  ⑦ 光照/材质/后处理 → 屏幕像素                               │   │
│  └─────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 二、FBX 导入详细设置

### 2.1 首次导入新角色

```
导入设置推荐（首次导入 Skeletal Mesh + 动画）：

【Mesh 设置】
✅ Import Mesh
✅ Import as Skeletal
⬜ Import Morph Targets（如有面部表情）
✅ Import Materials
✅ Import Textures

【骨骼设置】
Skeleton: 新建（首次导入）或选择已有骨骼（追加动画）

【变换设置】
Import Uniform Scale: 1.0（注意 Maya/Max 的单位是否与 UE 一致）
Import Translation: 0,0,0
Import Rotation: 0,0,0（如果 Maya 用了 Z-Up 导出）
               或 -90,0,0（如果 Maya 用 Y-Up 导出）

【动画设置】
✅ Import Animations
Animation Length: Exported Time（使用 FBX 中设定的时间范围）
✅ Import Custom Attribute（导入自定义属性为动画曲线）
✅ Preserve Local Transform（保持局部变换，防止根部偏移）

【LOD 设置】
LOD Import: Auto（自动处理 LOD）
```

### 2.2 追加导入动画

```
已有骨骼，只导入新动画时：

1. Content Browser → Import → 选择动画 FBX
2. 关键设置：
   Skeleton: 选择已有的 Skeleton 资产（！必须选择，否则创建新骨骼）
   ✅ Import Animations
   ⬜ Import Mesh（不需要重新导入 Mesh）
```

---

## 三、DCC 软件导出最佳实践

### Maya 导出设置

```
File → Export Selection → FBX 2020（或更高版本）

Animation 标签：
  ✅ Animation
  Bake Animation:
    ✅ Bake Animation（将所有约束烘焙为关键帧）
    Start/End: 设置正确的动画范围
    Step: 1（每帧都烘焙）
  ✅ Deformed Models → Skins（导出蒙皮）

Geometry 标签：
  ✅ Triangulate（建议三角化，UE 导入时也会三角化）
  ✅ Smooth Groups
  ✅ Preserve Edge Orientation

Advanced Options → Axis Conversion:
  Up Axis: Z（推荐，与 UE 一致）
  
Units:
  Automatic（或手动设置为 Centimeters，与 UE 单位一致）
```

### Blender 导出设置

```
File → Export → FBX (.fbx)

Transform:
  Scale: 1.0（Blender 默认 Scale=100，需注意）
  Apply Scalings: FBX Units Scale（推荐）
  Forward: -Y Forward（UE 的 X-Forward 对应 Blender 的 -Y）
  Up: Z Up
  ✅ Apply Transform（烘焙变换到骨骼）

Armature:
  ✅ Add Leaf Bones（添加末端骨骼，UE 需要）
  ⬜ Primary Bone Axis: X Primary（保持默认）

Animation:
  ✅ Baked Animation
  ✅ NLA Strips 或 All Actions（根据你的动画组织方式）
```

---

## 四、Live Link 实时流送

UE5 支持通过 **Live Link** 从 DCC 软件实时流送动画数据，无需导出 FBX：

```
Live Link 工作流：

Maya / MotionBuilder / Houdini
         │
         │（通过 Live Link 插件，局域网/本机）
         ▼
UE5 Live Link 插件（Window → Live Link）
         │
         ▼
Live Link Pose 节点（在 AnimBP 的 AnimGraph 中）
         │
         ▼
实时看到 DCC 中的动画效果！（无需导入等待）

用途：
• 动画师实时预览在引擎中的效果
• Mocap 数据实时预览（配合 OptiTrack/Vicon）
• 虚拟制片（Virtual Production）
```

---

## 五、延伸阅读

- 🔗 [Epic 官方：FBX 骨骼 Mesh 导入](https://dev.epicgames.com/documentation/en-us/unreal-engine/fbx-skeletal-mesh-pipeline-in-unreal-engine)
- 🔗 [虚幻引擎动画工作流指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-workflow-guides-and-examples-in-unreal-engine)
- 🔗 [Live Link 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/live-link-in-unreal-engine)
