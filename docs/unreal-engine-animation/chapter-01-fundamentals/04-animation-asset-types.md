# 1.4 动画资产类型全览

> **难度**：⭐⭐☆☆☆ | **阅读时间**：约 30 分钟

---

## 资产类型速查

| 资产类型 | 类名 | 图标颜色 | 核心用途 |
|---------|------|---------|--------|
| Animation Sequence | `UAnimSequence` | 🟢 绿色 | 基础关键帧动画 |
| Animation Montage | `UAnimMontage` | 🟡 黄色 | 可程序控制的分段动画 |
| Blend Space | `UBlendSpace` | 🔵 蓝色 | 多动画参数化混合 |
| Blend Space 1D | `UBlendSpace1D` | 🔵 蓝色 | 单轴混合 |
| Aim Offset | `UAimOffsetBlendSpace` | 🟣 紫色 | 瞄准方向叠加 |
| Pose Asset | `UPoseAsset` | 🟠 橙色 | 静态姿势/面部动画 |
| Animation Composite | `UAnimComposite` | ⚪ 白色 | 多段动画顺序拼接 |
| Anim Sequence Evaluator | 节点 | — | AnimGraph 中直接评估 |

---

## 一、Animation Sequence（动画序列）

### 1.1 核心概念

Animation Sequence 是最基础的动画资产，记录了角色骨骼在**时间轴**上的完整变换数据。

```
AnimSequence 数据结构（简化）：

时间轴: 0.0s ─────────────────────────────── 2.0s
                  （以 30fps 为例，共 60 帧）

骨骼轨道:
  root:     [T0: (0,0,0)] ─── [T30: (0,0,0)] ─── [T60: (0,0,0)]
  pelvis:   [T0: Rot_A]   ─── [T30: Rot_B]   ─── [T60: Rot_A]
  spine_01: [T0: Rot_C]   ─── [T30: Rot_D]   ─── [T60: Rot_C]
  ...

曲线轨道:
  MorphTarget_Smile: [T0: 0.0] ─── [T30: 1.0] ─── [T60: 0.0]

通知轨道:
  T15: FootstepNotify（左脚着地）
  T45: FootstepNotify（右脚着地）
```

### 1.2 关键属性

| 属性 | 路径 | 说明 |
|------|------|------|
| Rate Scale | Details → Rate Scale | 播放速率倍数，1.0 = 正常速度 |
| Loop | Details → Loop | 是否循环播放 |
| Enable Root Motion | Details → Root Motion | 提取根骨骼位移驱动角色移动 |
| Force Root Lock | Details → Root Motion | 锁定根骨骼，不受根运动影响 |
| Compression Settings | Details → Compression | 压缩算法（推荐 ACL） |
| Interpolation | Details → Interpolation | 关键帧插值方式 |

### 1.3 在编辑器中的操作

```
Animation Editor 操作：
• 双击 AnimSequence → 打开 Animation Editor
• 时间轴下方：Notifies 轨道（右键添加通知）
• 时间轴下方：Curves 轨道（管理动画曲线）
• 工具栏：Compression（一键应用压缩）
• 工具栏：ReImport（从原始 FBX 重新导入）
• 视口：可预览动画，支持调整播放速率
```

---

## 二、Animation Montage（动画蒙太奇）

### 2.1 核心概念

Montage 在 AnimSequence 基础上增加了：
- **Section（分段）**：将动画分为多个命名片段，可以跳转
- **Slot（槽）**：指定动画覆盖 AnimGraph 的哪个位置
- **独立的 Notify 轨道**：在 Montage 层面添加事件

```
Montage 结构示意：

Slot: DefaultSlot（或自定义槽名）
┌──────────────────────────────────────────────────────┐
│ Section: Startup  │ Section: Loop  │ Section: End    │
│ [引入动画 0.5s]   │ [主循环 1.0s]  │ [收尾动画 0.5s] │
│                   │                │                 │
│ [AnimSeq_A 片段]  │[AnimSeq_B循环] │[AnimSeq_C 片段] │
└──────────────────────────────────────────────────────┘

Notify 轨道:
  0.1s: Begin_SFX
  0.3s: Enable_Collision（NotifyState 开始）
  0.8s: Enable_Collision（NotifyState 结束）
  1.5s: End_SFX
```

### 2.2 C++ 播放控制

```cpp
// 播放 Montage
float PlayTime = AnimInstance->Montage_Play(
    AttackMontage,      // Montage 资产
    1.0f,               // 播放速率
    EMontagePlayReturnType::MontageLength,  // 返回值类型
    0.0f,               // 起始位置（秒）
    true                // 从开头停止其他 Montage
);

// 跳转到特定 Section
AnimInstance->Montage_JumpToSection(
    FName("Loop"),      // Section 名称
    AttackMontage       // 目标 Montage
);

// 停止播放
AnimInstance->Montage_Stop(0.25f, AttackMontage);  // 0.25s 淡出

// 暂停/继续
AnimInstance->Montage_Pause(AttackMontage);
AnimInstance->Montage_Resume(AttackMontage);

// 检查是否在播放
bool bIsPlaying = AnimInstance->Montage_IsPlaying(AttackMontage);

// 获取当前 Section
FName CurrentSection = AnimInstance->Montage_GetCurrentSection(AttackMontage);

// 绑定完成回调
FOnMontageEnded EndDelegate;
EndDelegate.BindUObject(this, &AMyCharacter::OnAttackMontageEnded);
AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
```

### 2.3 Montage Slot 与 AnimGraph 集成

```
AnimGraph 中：
[Locomotion State Machine]
        │
        ▼
[Slot Node: DefaultSlot]  ← Montage 插入点
        │
        ▼
[Output Pose]

当播放 Montage 时，Slot 节点自动混入 Montage 动画
当 Montage 停止时，Slot 节点透明传递下游姿势
```

---

## 三、Blend Space（混合空间）

### 3.1 1D Blend Space

```
1D Blend Space 示例（速度控制的运动）：

X 轴：Speed (0 → 600 cm/s)

0       150      300      450      600
│        │        │        │        │
Idle    Walk    Jog_L   Jog_H   Sprint

采样规则：Speed = 200 → 插值 Walk(0.33) + Jog_L(0.67)
```

### 3.2 2D Blend Space

```
2D Blend Space 示例（8方向运动）：

Y轴: Speed
600 │ Sprint_B  Sprint_BL/BR  Sprint_L/R  Sprint_FL/FR  Sprint_F
    │
300 │ Run_B    Run_BL/BR     Run_L/R     Run_FL/FR     Run_F
    │
150 │ Walk_B   Walk_BL/BR    Walk_L/R    Walk_FL/FR    Walk_F
    │
  0 │ Idle
    └─────────────────────────────────────────────────────────
      -180    -90       0        90       180
      X轴: Direction（移动方向，相对角色朝向）
```

### 3.3 Blend Space 高级配置

| 配置项 | 说明 |
|-------|------|
| **Snap to Grid** | 采样点对齐网格，避免人工精度误差 |
| **Smoothing Time** | 输入值变化的平滑时间（防止抖动）|
| **Target Weight Interpolation Speed** | 混合权重变化速度 |
| **Sync Group** | 同步组名称，用于多动画步频同步 |
| **Per Bone Override** | 指定骨骼级别的混合控制 |

---

## 四、Aim Offset（瞄准偏移）

Aim Offset 是特殊的 Blend Space，专为上半身瞄准方向叠加设计：

```
Aim Offset 坐标系：
Y轴: Pitch（俯仰）
 90° ─ Look_Up
  0° ─ Center（参考姿势）
-90° ─ Look_Down
      │
     -90°    0°    90°
      └──────┴──────┘
      X轴: Yaw（偏航）
      Look_L  Center  Look_R

使用方式：
• 9个采样点（左/中/右 × 上/中/下）
• 动画为相对参考姿势的"叠加动画"（Additive）
• 在 AnimGraph 中通过 Apply Additive 节点叠加到基础姿势
```

---

## 五、Pose Asset（姿势资产）

### 5.1 核心用途

Pose Asset 可以存储一组命名姿势，并通过**权重混合**在多个姿势间插值，特别适合**面部动画**：

```
Pose Asset 示例（面部表情库）：

姿势列表：
• "Smile_Mild"     权重 0.0→1.0
• "Smile_Full"     权重 0.0→1.0
• "Brow_Raise_L"   权重 0.0→1.0
• "Brow_Furrow"    权重 0.0→1.0
• "Eye_Blink_L"    权重 0.0→1.0
• "Eye_Blink_R"    权重 0.0→1.0

AnimGraph 中：[Pose Blender 节点]
  输入：Pose Asset + 各姿势权重（来自动画曲线或蓝图变量）
  输出：混合后的面部姿势
```

### 5.2 从 AnimSequence 创建 Pose Asset

```
操作步骤：
1. 打开包含表情关键帧的 AnimSequence
2. 在时间轴上找到想要保存的帧
3. 工具栏 → Create Asset → Create Pose Asset
4. 命名姿势，保存为 Pose Asset
5. 可以继续添加其他帧的姿势到同一个 Pose Asset
```

---

## 六、资产选择决策树

```
我的动画需求是？
│
├─ 单一连续动作（行走、跑步、待机）
│   └─→ Animation Sequence
│
├─ 需要程序控制（攻击、受击、翻滚）
│   └─→ Animation Montage
│
├─ 根据参数平滑过渡（速度、方向）
│   ├─ 单个参数 → Blend Space 1D
│   └─ 两个参数 → Blend Space 2D
│
├─ 上半身瞄准叠加
│   └─→ Aim Offset
│
├─ 面部表情、形态混合
│   └─→ Pose Asset
│
└─ 多段动画顺序播放（过场、连招）
    └─→ Animation Composite 或 Montage Section
```

---

## 七、延伸阅读

- 🔗 [Epic 官方：Animation Montage 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-montage-in-unreal-engine)
- 🔗 [Epic 官方：Blend Spaces 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/blend-spaces-in-unreal-engine)
- 🔗 [混合空间原理（知乎）](https://zhuanlan.zhihu.com/p/381967985)
- 🔗 [UE4/UE5 动画蒙太奇源码解析（知乎）](https://zhuanlan.zhihu.com/p/664971350)
