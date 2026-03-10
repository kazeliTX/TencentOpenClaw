# 第五章：Physics Asset & Ragdoll 布娃娃系统

> **前置要求**：第三章（约束）、第四章（刚体动力学）
> **学习目标**：掌握 PhAT 工具、Ragdoll 配置、布娃娃过渡与混合

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-physics-asset-overview.md](01-physics-asset-overview.md) | Physics Asset 总览与 PhAT 工具 | ⭐⭐⭐ |
| [02-body-setup.md](02-body-setup.md) | Body（物理体）设置详解 | ⭐⭐⭐⭐ |
| [03-constraint-setup.md](03-constraint-setup.md) | PhAT 约束配置 | ⭐⭐⭐⭐ |
| [04-ragdoll-activation.md](04-ragdoll-activation.md) | Ragdoll 激活流程 | ⭐⭐⭐⭐ |
| [05-ragdoll-blending.md](05-ragdoll-blending.md) | 布娃娃动画混合（BlendPhysics） | ⭐⭐⭐⭐⭐ |
| [06-get-up-animation.md](06-get-up-animation.md) | 起身动画（PoseSnapshot） | ⭐⭐⭐⭐⭐ |
| [07-hit-reaction.md](07-hit-reaction.md) | 受击反应（部分布娃娃） | ⭐⭐⭐⭐⭐ |
| [08-death-animation.md](08-death-animation.md) | 死亡动画过渡 | ⭐⭐⭐⭐ |
| [09-performance-optimization.md](09-performance-optimization.md) | Ragdoll 性能优化 | ⭐⭐⭐⭐ |
| [10-physics-asset-profiles.md](10-physics-asset-profiles.md) | Physics Asset Profile 管理 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_ragdoll_system.cpp](code/01_ragdoll_system.cpp) | 完整 Ragdoll 激活/恢复/受击系统 |
| [code/02_partial_ragdoll.cpp](code/02_partial_ragdoll.cpp) | 部分布娃娃（上半身命中反应） |
| [code/03_ragdoll_pooling.cpp](code/03_ragdoll_pooling.cpp) | Ragdoll 对象池（性能优化） |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/papers.md](references/papers.md) | 相关论文 |
| [references/blogs.md](references/blogs.md) | 博客与文章 |
