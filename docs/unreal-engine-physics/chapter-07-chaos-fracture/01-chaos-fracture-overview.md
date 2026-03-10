# 7.1 Chaos Fracture 系统总览

> **难度**：⭐⭐⭐☆☆

## 一、架构概览

```
Chaos Fracture 核心概念：

GeometryCollection（几何体集合）：
  破碎对象的核心资产
  包含层级化的碎片（Cluster Tree）
  每个叶节点 = 一块碎片 Mesh
  内部节点 = 碎片组（Cluster）

GeometryCollectionComponent：
  承载 GeometryCollection 的组件
  负责物理模拟 + 渲染
  替代传统的 StaticMeshComponent

破碎流程：
  1. 在编辑器 Fracture 工具中切割 Mesh → 生成 GeometryCollection
  2. 配置 Cluster 层级（哪些碎片先分开，哪些后分开）
  3. 配置连接强度（Damage Threshold）
  4. 运行时通过 Field 或直接调用 API 触发破碎

Chaos Fracture vs 旧版 Apex Destruction：
  Apex：NVIDIA 库，UE4 时代，UE5 已废弃
  Chaos：Epic 自研，原生集成，支持层级破碎
  主要改进：无限层级破碎/Field System/GPU 加速渲染
```

## 二、GeometryCollection 层级结构

```
层级树示例（一面墙）：
  Root
  ├── Cluster_A（上半部分）
  │   ├── Piece_01
  │   ├── Piece_02
  │   └── Piece_03
  └── Cluster_B（下半部分）
      ├── Piece_04
      ├── Piece_05
      └── Piece_06

破碎顺序：
  Level 0（Cluster）先断裂：墙分成上下两大块
  Level 1（Piece）再断裂：每块再碎成小碎片
  
  好处：可以控制破碎的粒度和时序
  应用：子弹打墙 → 先局部裂开（Cluster 破碎），
        爆炸 → 整体崩碎（所有 Level 破碎）
```

## 三、延伸阅读

- 📄 [7.2 GeometryCollection 资产](./02-geometry-collection.md)
- 🔗 [Chaos Fracture Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-destruction-overview)
