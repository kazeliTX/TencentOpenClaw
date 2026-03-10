# 10.1 流体系统总览

> **难度**：⭐⭐⭐⭐☆

## UE5 流体方案对比

```
方案                   适合场景              性能     真实度   版本要求
──────────────────────────────────────────────────────────────────────
Water Plugin          大型海洋/湖泊/河流     ★★★★    ★★★★    UE5+
Niagara Fluids        局部流体特效(溅水)     ★★★     ★★★★★   UE5+
流体材质(波纹贴图)    纯视觉水面             ★★★★★   ★★★      任何版本
Chaos Fluid           实验性 SPH 流体        ★       ★★★★★   UE5.1+
Buoyancy Component    物体浮力              ★★★★★   ★★★     UE5+

选择原则：
  开放世界水体 → Water Plugin（Ocean/Lake/River）
  爆炸/碰撞溅水特效 → Niagara Fluids
  小水坑/水面反射 → 流体材质 + Normal Map 动画
  真实流体模拟（科研/高端） → Chaos Fluid（不推荐生产）
  浮力 → BuoyancyComponent（配合 Water Plugin）
```
