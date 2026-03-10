# 第三章：物理材质与约束

> **前置要求**：第一章（刚体基础）、第二章（碰撞系统）
> **学习目标**：掌握物理材质参数调优，以及各类约束（铰链、弹簧、绳索）的使用

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-physics-material-overview.md](01-physics-material-overview.md) | 物理材质总览与创建 | ⭐⭐⭐ |
| [02-friction-restitution.md](02-friction-restitution.md) | 摩擦力与弹性（Friction & Restitution） | ⭐⭐⭐⭐ |
| [03-surface-type.md](03-surface-type.md) | 表面类型（SurfaceType）与脚步音效/粒子 | ⭐⭐⭐ |
| [04-constraint-overview.md](04-constraint-overview.md) | 物理约束总览（Constraint 类型） | ⭐⭐⭐⭐ |
| [05-hinge-constraint.md](05-hinge-constraint.md) | 铰链约束（门/旋转轴）| ⭐⭐⭐⭐ |
| [06-spring-constraint.md](06-spring-constraint.md) | 弹簧约束（弹性连接）| ⭐⭐⭐⭐⭐ |
| [07-prismatic-constraint.md](07-prismatic-constraint.md) | 滑动约束（活塞/电梯）| ⭐⭐⭐⭐ |
| [08-joint-limits.md](08-joint-limits.md) | 约束限制（Angular/Linear Limits） | ⭐⭐⭐⭐⭐ |
| [09-constraint-motor.md](09-constraint-motor.md) | 约束驱动器（Motor/Drive） | ⭐⭐⭐⭐⭐ |
| [10-rope-chain.md](10-rope-chain.md) | 绳索与链条（Cable Component / 物理绳）| ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_physics_material_usage.cpp](code/01_physics_material_usage.cpp) | 运行时物理材质创建与替换 |
| [code/02_constraint_door.cpp](code/02_constraint_door.cpp) | 铰链门：约束创建/弹簧回弹/电机驱动 |
| [code/03_rope_chain.cpp](code/03_rope_chain.cpp) | 物理绳索/链条运行时创建与参数调优 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/papers.md](references/papers.md) | 相关论文 |
| [references/blogs.md](references/blogs.md) | 博客与文章 |
