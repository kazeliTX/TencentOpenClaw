# 10.9 流体性能优化

> **难度**：⭐⭐⭐⭐☆

## 一、Water Plugin 性能控制

```
WaterMesh 细分：
  WaterBodyActor → Details → Water Mesh：
    LOD Scale：控制水面 Mesh 细节（越大越细腻越慢）
    Far Distance Mesh Scale Factor：远处网格缩放
  
  建议：近处 LOD Scale=1.0，远处降至 0.25

Gerstner Wave 层数：
  波浪层数越多越真实，但 Shader 计算线性增长
  移动/主机：4 层
  PC：8~16 层

水体可视距离：
  对不可见水体（背后/水下）禁用 Tick
  UWaterBodyComponent::SetWaterBodyRenderingEnabled(false)
```

## 二、Niagara Fluids 性能

```
Grid Resolution 对性能影响（GPU 时间）：
  32×32：  ~0.2ms
  64×64：  ~0.8ms
  128×128：~3.2ms（约 4x 规律）
  256×256：~13ms（不推荐实时）

优化策略：
  1. 同屏 Niagara Fluid 数量 ≤ 2（主机）/ ≤ 4（高端PC）
  2. 不在交互时降低分辨率（动态 Resolution）
  3. 水体静止时 Pause Niagara System
  
CVar：
  fx.NiagaraFluids.MaxResolution 64  ← 全局最大分辨率
  fx.NiagaraFluids.Enable 0          ← 关闭所有流体（性能测试）
```
