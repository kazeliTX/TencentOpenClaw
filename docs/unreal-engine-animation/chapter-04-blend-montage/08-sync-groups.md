# 4.8 Sync Groups 动画同步

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 20 分钟

---

## 一、同步组解决的问题

```
问题场景：Walk → Run 过渡时脚步不同步

Walk 动画：左脚在 0.0s 落地，右脚在 0.5s 落地（步频 1Hz）
Run 动画：  左脚在 0.0s 落地，右脚在 0.3s 落地（步频 1.67Hz）

在混合过渡中（混合权重各 50%）：
  两个动画的相位不同步
  → 一只脚飞起来，另一只脚踩地 → 严重滑步

Sync Groups 解决方案：
  将所有运动动画放入同一个 Sync Group
  "Leader"（权重最高的动画）的时间位置为基准
  所有 "Follower" 动画同步到 Leader 的归一化时间
  → 混合时所有动画的步态相位保持一致
```

---

## 二、配置 Sync Groups

```
方法 1：在 Blend Space 中配置（推荐）
  打开 BS_Locomotion
  资产设置 → Sync Group Name: SyncGroupLoco
  Sync Group Role: Can Be Leader（默认）
  → Blend Space 内所有采样点自动同步

方法 2：在 AnimGraph 节点上配置
  Sequence Player 节点 → Details → Sync Group
    Group Name: SyncGroupLoco
    Group Role: Can Be Leader / Always Leader / Always Follower

方法 3：在 State Machine 节点上配置（UE5.3+）
  State Machine → 在状态内的 Sequence Player → 设置 Sync Group
```

### Sync Group Role 含义

| Role | 说明 | 适用场景 |
|------|------|---------|
| `Can Be Leader` | 权重最高时成为 Leader（默认）| 大多数运动动画 |
| `Always Leader` | 永远作为 Leader | 主循环动画（如 Idle）|
| `Always Follower` | 永远跟随 Leader | 叠加层动画 |
| `Transition Leader` | 过渡时作为 Leader | 特殊过渡动画 |
| `Transition Follower` | 过渡时跟随 | 特殊过渡动画 |

---

## 三、同步组的工作原理

```
每帧同步过程：

1. 收集同一 Sync Group 内所有活跃动画及其权重
2. 权重最高的那个成为 Leader（如果没有 Always Leader）
3. Leader 正常按 DeltaTime 推进时间
4. 所有 Follower 将自身时间映射到 Leader 的归一化时间：
   
   Follower 时间 = Leader 归一化时间 × Follower 动画总时长
   
   例：Leader（Walk 1.0s）归一化时间 = 0.3
       Follower（Run 0.6s）时间 = 0.3 × 0.6 = 0.18s

5. 效果：两个动画的步态相位始终对齐
```

---

## 四、Sync Marker（精确同步标记）

UE5 引入了 Sync Marker，比基于时间的同步更精确：

```
Sync Marker 工作原理：
  在动画的时间轴上标记"脚落地"事件
  Sync Groups 根据标记对齐，而不是简单的时间比例

配置：
  1. 打开 Walk 动画 → Notifies 轨道
  2. 右键 → Add Sync Marker → 命名：LeftFoot
     在左脚落地的精确帧放置标记
  3. 右键 → Add Sync Marker → 命名：RightFoot
     在右脚落地的精确帧放置标记
  4. 对所有同一 Sync Group 的动画做同样的操作

效果：
  Walk 和 Run 在对应的脚落地帧精确对齐
  比时间比例同步精准得多
```

---

## 五、延伸阅读

- 🔗 [Sync Groups 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-sync-groups-in-unreal-engine)
- 🔗 [Sync Marker 教程](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-sync-groups-in-unreal-engine#syncmarkers)
