# 7.9 破碎 LOD 与性能优化

> **难度**：⭐⭐⭐⭐☆

## 一、GC 性能参数

```
关键性能控制参数（GC Details → Chaos Physics）：

Minimum Mass Threshold（最小质量阈值）：
  质量 < 此值的碎片自动移除
  = 1.0 kg（推荐起始值）
  太小 → 大量微小碎片积累，性能崩溃
  太大 → 破碎效果不细腻

Maximum Sleep Time：
  碎片静止多久后进入 Sleep（秒）
  = 1.0（默认）→ 1 秒静止即 Sleep
  Sleep 碎片几乎不消耗 CPU

Cluster Group Index：
  相同 Index 的 GC 作为一个 Cluster 处理
  同屏多块同材质的砖 → 共享 Cluster，降低管理开销

Max Level：
  运行时允许破碎的最大层级深度
  = 1 → 只能破碎到 Level 1（大块），不会进一步细碎
  性能受限场景下降低此值
```

## 二、性能预算估算

```
同屏同时活跃 GC 数量建议：

低端（移动/主机 Last Gen）：
  活跃碎片数 < 200
  同时破碎 GC < 3

中端（PS5/高端 PC）：
  活跃碎片数 < 1000
  同时破碎 GC < 10

高端（RTX PC + Chaos 异步）：
  活跃碎片数 < 5000
  同时破碎 GC < 30

碎片数控制策略：
  1. 限制 GC 的总碎片数（Fracture 工具时控制 Site Count）
  2. 设置 Minimum Mass Threshold 自动移除小碎片
  3. 远处 GC 降低 Max Level（不细碎）
  4. 超距离的 GC 切换回 StaticMesh（不参与破碎）

控制台命令：
  p.gc.MaxSimultaneousDestructionActors 10  ← 最大同时破碎 Actor 数
  p.Chaos.Solver.Iterations 2              ← 降低求解迭代（牺牲精度换性能）
  p.Chaos.Cluster.UseConnectivityGraph 0   ← 关闭连接图（加速但破碎不传播）
```

## 三、延伸阅读

- 📄 [7.10 破碎常见模式](./10-fracture-common-patterns.md)
