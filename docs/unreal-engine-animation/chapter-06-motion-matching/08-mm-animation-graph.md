# 6.8 Motion Matching 在 AnimGraph 中的配置

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、Motion Matching 节点

```
AnimGraph 中添加 Motion Matching 节点（搜索 "Motion Matching"）

节点参数：
  Database:          PSD_Locomotion（你的 Pose Search Database）
  Trajectory Source: Character Trajectory Component（角色组件）
  Blend Time:        0.2s（切换动画帧时的混合时间）
  Max Cost:          3.0（最大允许的匹配 Cost，超过则不切换）
  
  Pose Jump Threshold Cost: 1.0
    → Cost 差异超过此值才切换到新帧（防止频繁小幅切换）
    
  Min Time Between Pose Reselects: 0.1s
    → 两次搜索之间的最小间隔（控制搜索频率）
```

---

## 二、完整的 MM AnimGraph 结构

```
推荐的 AnimGraph 层级：

[Motion Matching]       ← 基础运动（Locomotion）
      │
[Distance Matching]     ← 可选，精确停步/起步
      │
[Orientation Warping]   ← 可选，方向扭曲（角速度平滑）
      │
[Stride Warping]        ← 可选，步幅扭曲（速度自适应）
      │
[Layered Blend Per Bone]← 上半身叠加（武器、Aim等）
  Base:   ← 下半身（MM 输出）
  Blend:  ← 上半身 Slot
      │
[Apply Aim Offset]      ← 瞄准偏移
      │
[Full Body IK]          ← 脚步 IK
      │
[Output Pose]
```

---

## 三、Distance Matching（距离匹配）

```
Distance Matching 解决起步/停步的精确控制：

问题：
  角色停步时，MM 可能选到"减速中段"的帧
  导致停步距离和实际距离不符（滑步感）

Distance Matching 解决：
  为停步动画添加"剩余距离曲线"（Remaining Distance Curve）
  AnimGraph 中通过当前剩余距离直接定位到动画中的对应帧
  → 动画的减速节奏精确匹配角色的减速距离

配置：
  1. 打开停步动画 → Add Curve → "Distance"
  2. 手动标记每帧的剩余停步距离（cm）
  3. AnimGraph → Distance Matching 节点
     Curve Name: "Distance"
     Distance: StopRemainingDistance（来自 AnimInstance）
```

---

## 四、Orientation Warping（方向扭曲）

```
Orientation Warping 让角色在斜向移动时看起来更自然：

问题：
  斜向 45° 移动时，MM 可能选到"正前方"动画
  骨盆旋转不足，看起来像在侧滑

Orientation Warping：
  自动将下半身动画旋转到实际移动方向
  上半身保持朝向目标（不旋转）
  → 斜向移动时脚的朝向正确，上身保持面朝前方

配置：
  Orientation Warping 节点 → Locomotion Direction
  Lower Body Rotation Axis: Z（只旋转 Z 轴）
  Spine Bones: spine_01, spine_02, spine_03（上半身反向旋转补偿）
```

---

## 五、延伸阅读

- 🔗 [Motion Matching in UE5 官方教程](https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-matching-in-unreal-engine)
- 🔗 [Distance Matching 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/distance-matching-in-unreal-engine)
- 🔗 [Lyra MM 实现参考](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-in-lyra-sample-game-in-unreal-engine)
