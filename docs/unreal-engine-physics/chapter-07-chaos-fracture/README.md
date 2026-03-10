# 第七章：Chaos 破碎系统

> **前置要求**：第一章（Chaos 架构）、第四章（刚体动力学）
> **学习目标**：掌握 GeometryCollection、Fracture 工具、Field System 与破碎性能优化

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-chaos-fracture-overview.md](01-chaos-fracture-overview.md) | Chaos Fracture 系统总览 | ⭐⭐⭐ |
| [02-geometry-collection.md](02-geometry-collection.md) | GeometryCollection 资产详解 | ⭐⭐⭐⭐ |
| [03-fracture-tools.md](03-fracture-tools.md) | Fracture 工具（Voronoi/平面/层级）| ⭐⭐⭐⭐ |
| [04-field-system.md](04-field-system.md) | Field System（力场/应变场）| ⭐⭐⭐⭐⭐ |
| [05-break-threshold.md](05-break-threshold.md) | 破碎阈值与连接强度 | ⭐⭐⭐⭐ |
| [06-anchor-fields.md](06-anchor-fields.md) | 锚定场（Anchor Fields）| ⭐⭐⭐⭐ |
| [07-fracture-events.md](07-fracture-events.md) | 破碎事件与回调 | ⭐⭐⭐⭐ |
| [08-procedural-fracture.md](08-procedural-fracture.md) | 程序化破碎运行时 | ⭐⭐⭐⭐⭐ |
| [09-fracture-lod.md](09-fracture-lod.md) | 破碎 LOD 与性能优化 | ⭐⭐⭐⭐ |
| [10-fracture-common-patterns.md](10-fracture-common-patterns.md) | 破碎常见实现模式 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_fracture_trigger.cpp](code/01_fracture_trigger.cpp) | 运行时触发破碎 + 事件回调 |
| [code/02_field_system.cpp](code/02_field_system.cpp) | 力场/应变场动态施加 |
| [code/03_destruction_manager.cpp](code/03_destruction_manager.cpp) | 破碎管理器（对象池/预算控制）|

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/papers.md](references/papers.md) | 相关论文 |
| [references/blogs.md](references/blogs.md) | 博客与文章 |
