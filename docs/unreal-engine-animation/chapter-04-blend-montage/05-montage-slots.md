# 4.5 Montage Slot 与混合层

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 20 分钟

---

## 一、Slot 的作用

Montage Slot（动画槽位）定义了 Montage 动画插入 AnimGraph 的**位置和方式**：

```
没有 Slot：Montage 无法播放！
AnimGraph 中的 Slot 节点 = Montage 的"播放端口"

数据流：
  State Machine 输出姿势
        │
        ▼
  [Slot: UpperBody]  ← Montage 从这里注入
        │
        ▼（如果没有活跃 Montage：直接 pass-through）
        ▼（如果有活跃 Montage：混合 Montage 姿势）
  Output Pose
```

---

## 二、内置 Slot 类型

```
UE 默认提供两个 Slot：

DefaultSlot（全身槽）：
  覆盖所有骨骼
  用途：互动动作、死亡动画、技能释放

UpperBody（上半身槽）：
  通过 Layered Blend Per Bone 只覆盖上半身
  用途：攻击动画（腿部继续走路，上半身攻击）

自定义 Slot：
  Skeleton 编辑器 → Anim Slot Manager → 添加自定义 Slot
  例：FaceSlot（只控制面部骨骼）
      WeaponSlot（只控制武器手）
```

---

## 三、在 AnimGraph 中配置 Slot

```
完整的上半身攻击配置：

[State Machine（下半身运动）]
        │
        ▼
[Layered Blend Per Bone]
  Base Pose: ← State Machine
  Blend Pose 0: ←── [Slot: UpperBody]  ← 攻击 Montage 注入点
    Layer: spine_03（上半身起始骨骼）
    Alpha: 1.0
        │
        ▼
[Slot: DefaultSlot]    ← 全身 Montage 注入点（如互动动作）
        │
        ▼
[Output Pose]
```

---

## 四、Slot 权重与混合

```
Slot 混合规则：
  没有活跃 Montage：Base Pose（State Machine）权重 1.0
  Montage 播放中：Base Pose + Montage 按配置混合
  Montage 淡出：权重从 1.0 渐变回 0.0（Base Pose 恢复）

混合参数（在 Montage 资产中配置）：
  Blend In Time:  0.15s（Montage 开始时淡入时长）
  Blend Out Time: 0.25s（Montage 结束时淡出时长）
  Blend Out Trigger Time: -1（-1 = 自动，即结束前 BlendOutTime 开始淡出）
```

---

## 五、多 Montage 同时播放

```
可以同时播放多个 Montage，但必须使用不同的 Slot！

示例：角色同时：
  1. 播放攻击 Montage（UpperBody Slot）
  2. 播放对话表情 Montage（FaceSlot）

AnimGraph 需要有两个对应 Slot 节点：
  [Slot: UpperBody]  ← 攻击注入
  [Slot: FaceSlot]   ← 表情注入

如果两个 Montage 使用同一个 Slot：
  后播放的会打断（中断）前一个
  这是正常行为，可用于连击系统
```

---

## 六、延伸阅读

- 📄 [4.4 Animation Montage 完全指南](./04-animation-montage.md)
- 🔗 [Montage Slot Manager](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-slots-in-unreal-engine)
