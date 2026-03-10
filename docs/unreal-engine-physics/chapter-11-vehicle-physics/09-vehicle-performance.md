# 11.9 性能优化

> **难度**：⭐⭐⭐⭐☆

```
Chaos Vehicle 性能建议：

单辆车物理开销：
  简单车辆（4轮/无悬挂动画）：~0.3ms
  完整配置（悬挂/扭矩/差速）：~0.8~1.5ms
  同屏 20 辆：~10~30ms → 压力较大
  
优化策略：
  1. 远距离车辆：SetSimulatePhysics(false) + 线性插值位置
     距离 > 5000cm → 关闭物理，用服务端同步位置
     
  2. 停止的车辆：设置 Sleep（零速度时自动入睡）
     ChaosVehicle.SleepCounterThreshold = 20（默认）
     
  3. 减少同时激活车辆数：
     玩家附近 300m：完整物理
     300~600m：简化物理（降低 SubstepCount）
     >600m：纯位置同步（服务端广播）
     
  4. SubstepCount：
     默认 3（稳定但慢）
     2（中等平衡）
     1（快但可能穿地）

CVar：
  p.Chaos.Vehicle.SimulationEnabled 1   ← 全局开关
  p.Chaos.Vehicle.MaxSubsteps 2         ← 降低子步数
```
