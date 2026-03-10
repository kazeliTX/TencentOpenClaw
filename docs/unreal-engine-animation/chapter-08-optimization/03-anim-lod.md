# 8.3 动画系统 LOD（AnimBP LOD）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、AnimBP LOD 的核心思想

```
即使骨骼数减少了，AnimGraph 的节点复杂度仍然影响 CPU：
  LOD0：完整 AnimGraph（IK + FBIK + Aim Offset + 布料 + ...)
  LOD1：简化 AnimGraph（无 IK，无 Aim Offset）
  LOD2：极简 AnimGraph（单一动画序列）
  LOD3：静止帧（不更新）

每个 AnimGraph 节点都有 LOD Threshold 属性：
  设置该节点在 >= 某 LOD 时跳过（不计算）
```

---

## 二、节点 LOD Threshold 配置

```
在 AnimGraph 中选中任意节点：
  Details 面板 → LOD Threshold

常见节点的推荐 LOD 阈值：

节点                        推荐阈值    说明
───────────────────────────────────────────────────────
Two-Bone IK（脚步）          LOD 1       LOD2+ 禁用
Two-Bone IK（手部）          LOD 1       LOD2+ 禁用
Aim Offset                  LOD 1       LOD2+ 禁用
Full Body IK                LOD 0       只在极近距离使用
Layered Blend Per Bone      LOD 2       LOD3+ 禁用
Apply Mesh Space Additive   LOD 1       LOD2+ 禁用
FABRIK                      LOD 1       LOD2+ 禁用
Look At                     LOD 2       LOD3+ 禁用
AnimDynamics                LOD 1       LOD2+ 禁用
Rigid Body                  LOD 1       LOD2+ 禁用

设置方式：
  节点 → Details → LOD Threshold: 1
  → 意思：当 MeshComponent.ForcedLodModel >= 1 时（即 LOD1 及以上），跳过该节点
```

---

## 三、LOD 专用状态机（推荐架构）

```
推荐架构：为不同 LOD 创建独立的 State Machine

AnimGraph 结构：

LOD 0/1:
  [Full Locomotion State Machine]  ← 完整 32 状态
        │
  [IK Layer]                       ← 脚步 + 手部 IK
        │
  [Aim Offset]
        │
  [Additive Layers]

LOD 2:
  [Simple Locomotion SM]           ← 只有 Walk/Run/Idle 3 状态
        │
  [Output Pose]

LOD 3+:
  [Single Sequence（A_Idle）]      ← 单一 Idle 动画

实现方式（Blend 切换）：
  用 LOD 驱动的 Blend 节点选择哪套管线
  或使用 Linked Anim Graph 按 LOD 加载不同子图
```

---

## 四、OnlyTickPoseWhenRendered（最重要的优化）

```
屏幕外的角色不需要更新动画！

设置：
  SkeletalMeshComponent → Optimization → Only Tick Pose When Rendered: ✅

效果：
  角色离开屏幕（视锥体剔除）→ 动画更新完全停止
  角色重新进入视野 → 动画立即恢复
  
  对于不在视野内的大量 NPC：完全零动画开销！

注意：
  某些特效/音效依赖动画事件（AnimNotify）时，
  屏幕外停止更新可能导致特效/音效错过触发
  解决：将这些逻辑移到 C++/蓝图，不依赖 AnimNotify

启用时机：
  大多数 NPC：✅ 始终开启
  玩家角色：⚠️ 通常不开启（镜头变化后立即需要正确姿势）
  关键 NPC（剧情/BOSS）：⚠️ 视情况决定
```

---

## 五、Pose Caching（姿势缓存）

```
如果多个地方需要相同的姿势（如双手持同一把枪的两个效应器）
使用 Cached Pose 节点避免重复计算：

[Base Locomotion]
        │
[Cached Pose: LocoCache]
  ↙                ↘
[Left Hand IK]  [Right Hand IK]
（两者共享同一基础姿势，不重复计算）
```

---

## 六、延伸阅读

- 🔗 [Anim LOD 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine#lodsettings)
- 🔗 [AnimBP 优化最佳实践](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine)
