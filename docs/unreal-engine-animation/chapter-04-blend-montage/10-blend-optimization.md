# 4.10 混合系统性能优化

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 20 分钟

---

## 一、混合系统的性能开销

```
每类混合节点的相对开销：

节点类型                    相对开销   备注
──────────────────────────────────────────────────
Blend Space（2轴，9点）       中        每帧最多混合 4 个采样点
Aim Offset（9点）              中        Mesh Space 转换有额外开销
Layered Blend Per Bone        低-中      遍历骨骼链
Apply Mesh Space Additive     中-高      空间转换计算
Animation Montage（活跃时）   低         Slot 注入开销小
Motion Warping                低         仅修改 Root Motion，不影响 FK
Sync Groups                   极低       仅时间同步，无额外骨骼计算
```

---

## 二、关键优化策略

### 2.1 减少 Blend Space 采样点

```
采样点越多 = 每帧计算越多（最多 4 点双线性插值）

优化：
  • 移动端：3~5 个采样点（Idle + 前后左右）
  • PC/主机：9~11 个采样点（8方向 + Idle）
  • 超过 15 个采样点没有明显视觉收益

测试方法：
  减少采样点 → 在编辑器中预览 → 检查是否有明显混合接缝
  没有接缝 = 可以保持精简
```

### 2.2 Montage Blend Time 控制

```
Montage Blend Time 影响同时活跃的动画数量：

攻击连段 Blend Time: 0.1~0.15s（快速响应）
→ 短时间内只有 2 个 Montage 同时活跃

大幅度动作 Blend Time: 0.2~0.3s
→ 稍长一些，但不要超过 0.5s

极短 Blend（0.05s 以内）：
→ 实际上几乎等同于立即切换，考虑用 Inertialization 替代
```

### 2.3 禁用不必要的 Mesh Space Additive

```
Mesh Space Additive 需要将每根骨骼从 Local Space 转换到 Mesh Space
开销约为普通叠加的 2~3 倍

只在真正需要世界方向一致性时才用 Mesh Space：
  必须用：Aim Offset（瞄准方向）
  可以用 Local：呼吸叠加、受伤前倾、表情
```

### 2.4 BlendSpace 的 LOD Threshold

```
远处角色不需要精细的方向混合：

配置：
  Blend Space Player 节点 → Details → LOD Threshold: 1
  → LOD2+ 时跳过 Blend Space，使用默认单一动画

更激进：
  LOD2：用单个 Walk_Fwd 替代 BlendSpace
  LOD3：用单帧参考姿势（静止）
```

---

## 三、Motion Warping 性能

```
Motion Warping 开销极低：
  仅修改根骨骼的运动路径（不影响其他骨骼）
  每帧计算：简单的插值计算，O(1)

注意：
  • 同时活跃的 Warp Target 数量不影响性能（只有少数几个）
  • 真正的开销在于 Root Motion 的物理计算，但这是必须的
```

---

## 四、优化数据参考

```
100个角色场景的混合系统开销（参考值）：

完整混合（BS + Aim Offset + Additive × 2）：
  ~1.5ms（Worker Thread，LOD0）

LOD1（禁用 Aim Offset + 简化 BS）：
  ~0.8ms

LOD2（单一动画，无 BS）：
  ~0.3ms

LOD3（仅骨架更新，极简）：
  ~0.1ms

结论：
  LOD 系统是最有效的混合优化手段
  不要为了"保险"在所有 LOD 都开全特性
```

---

## 五、延伸阅读

- 🔗 [Animation Optimization 官方](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine)
- 🔗 [UE5 Blend Space 性能分析（知乎）](https://zhuanlan.zhihu.com/p/381967985)
