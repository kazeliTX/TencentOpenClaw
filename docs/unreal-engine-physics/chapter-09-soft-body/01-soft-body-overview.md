# 9.1 软体物理总览

> **难度**：⭐⭐⭐⭐☆

## 一、UE5 软体物理方案对比

```
方案             适合场景            性能      交互性    实现难度
─────────────────────────────────────────────────────────────
UCableComponent  视觉装饰绳/电线     ★★★★★   低（不可爬）  ⭐⭐
Chaos Cloth      布料（已在Ch06介绍） ★★★★    中           ⭐⭐⭐
Physics Rope     可交互绳索/链条      ★★★     高（可爬攀）  ⭐⭐⭐⭐
Chaos Soft Body  软包/果冻/橡皮体     ★★★     中           ⭐⭐⭐⭐⭐
Constraint Chain 刚性链（铁链）       ★★★     高           ⭐⭐⭐⭐
```

## 二、选择指南

```
需要可以攀爬的绳子？
  → Physics Rope（Physics Constraint 链条）

纯视觉电线/垂落绳装饰？
  → UCableComponent（最省性能）

果冻/软包/弹性体？
  → Chaos Soft Body（UE5.2+ 实验性功能）
  → 或用 Chaos Cloth 近似（薄软包）

铁链/金属链条？
  → StaticMesh Physics Body 链条（Constraint Chain）
  → 或 UCableComponent（纯装饰）
```
