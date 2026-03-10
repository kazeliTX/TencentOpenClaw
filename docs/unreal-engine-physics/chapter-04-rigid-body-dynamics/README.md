# 第四章：刚体动力学

> **前置要求**：第一章（物理基础）、第三章（约束）
> **学习目标**：掌握力/冲量/扭矩/速度/质心的完整控制，实现各类物理效果

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-forces-overview.md](01-forces-overview.md) | 力的种类与施加方式 | ⭐⭐⭐ |
| [02-impulse-velocity.md](02-impulse-velocity.md) | 冲量与速度控制 | ⭐⭐⭐⭐ |
| [03-torque-angular.md](03-torque-angular.md) | 力矩与角速度 | ⭐⭐⭐⭐ |
| [04-explosion-force.md](04-explosion-force.md) | 爆炸力场（RadialForce） | ⭐⭐⭐⭐ |
| [05-gravity-buoyancy.md](05-gravity-buoyancy.md) | 重力变体与浮力 | ⭐⭐⭐⭐ |
| [06-force-at-location.md](06-force-at-location.md) | 点施力（AddForceAtLocation）| ⭐⭐⭐⭐⭐ |
| [07-velocity-control.md](07-velocity-control.md) | 速度直接控制（SetVelocity） | ⭐⭐⭐⭐ |
| [08-center-of-mass-advanced.md](08-center-of-mass-advanced.md) | 质心高级应用 | ⭐⭐⭐⭐⭐ |
| [09-physical-simulation-events.md](09-physical-simulation-events.md) | 物理模拟事件与状态机 | ⭐⭐⭐⭐ |
| [10-kinematic-driven.md](10-kinematic-driven.md) | Kinematic 驱动与混合模式 | ⭐⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_force_impulse.cpp](code/01_force_impulse.cpp) | 力/冲量/扭矩完整 API 示例 |
| [code/02_explosion_system.cpp](code/02_explosion_system.cpp) | 爆炸系统：RadialForce + 破片 |
| [code/03_kinematic_blend.cpp](code/03_kinematic_blend.cpp) | Kinematic/Physics 混合控制 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/papers.md](references/papers.md) | 相关论文 |
| [references/blogs.md](references/blogs.md) | 博客与文章 |
