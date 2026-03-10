# 9.9 软体物理性能优化

> **难度**：⭐⭐⭐⭐☆

```
性能预算参考：

UCableComponent（每根）：
  NumSegments=8, SolverIterations=4 → ~0.02ms/根
  同屏 100 根：约 2ms（可接受）

物理绳索链条（每根 16 节）：
  ≈ 0.5~1ms/根
  同屏建议 ≤ 8 根交互绳索

Chaos Soft Body（实验）：
  每个软体 ≈ 2~10ms（取决于粒子数）
  同屏建议 ≤ 2 个

优化策略：
  1. 远处绳索：减少 NumSegments（自动 LOD）
  2. 非可见绳索：暂停物理（SuspendPhysics）
  3. 物理绳索超出视野：SetSimulatePhysics(false)
  4. 软体不活跃时：SetEnableGravity(false) + 冻结

CVar 控制：
  p.CableComponent.MaxSegments 8  ← 全局限制 Cable 最大段数
```
