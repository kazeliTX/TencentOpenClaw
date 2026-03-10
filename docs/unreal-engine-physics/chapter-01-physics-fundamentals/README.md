# 第一章：物理系统基础

> **前置要求**：了解 UE 基本概念（Actor、Component、蓝图）
> **学习目标**：理解 UE 物理引擎架构、核心概念与基本使用流程

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-physics-engine-overview.md](01-physics-engine-overview.md) | 物理引擎总览（PhysX→Chaos）| ⭐⭐⭐ |
| [02-chaos-architecture.md](02-chaos-architecture.md) | Chaos 引擎架构 | ⭐⭐⭐⭐ |
| [03-rigid-body-basics.md](03-rigid-body-basics.md) | 刚体基础概念 | ⭐⭐⭐ |
| [04-physics-actor-setup.md](04-physics-actor-setup.md) | 物理 Actor 配置 | ⭐⭐⭐ |
| [05-mass-inertia.md](05-mass-inertia.md) | 质量与惯性 | ⭐⭐⭐⭐ |
| [06-gravity-damping.md](06-gravity-damping.md) | 重力与阻尼 | ⭐⭐⭐ |
| [07-physics-tick.md](07-physics-tick.md) | 物理帧更新机制 | ⭐⭐⭐⭐ |
| [08-substepping.md](08-substepping.md) | 子步进（Substepping）| ⭐⭐⭐⭐ |
| [09-physics-scene.md](09-physics-scene.md) | 物理场景与世界设置 | ⭐⭐⭐ |
| [10-debug-visualization.md](10-debug-visualization.md) | 物理调试可视化 | ⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_enable_physics.cpp](code/01_enable_physics.cpp) | 运行时启用/禁用物理 |
| [code/02_physics_query.cpp](code/02_physics_query.cpp) | 物理查询基础 |
| [code/03_substepping_demo.cpp](code/03_substepping_demo.cpp) | 子步进回调示例 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/papers.md](references/papers.md) | 相关论文 |
| [references/blogs.md](references/blogs.md) | 博客与文章 |
