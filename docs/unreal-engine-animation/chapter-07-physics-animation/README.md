# 第七章：物理动画与布娃娃系统

> **前置要求**：已完成第二章（骨骼系统）、第五章（IK 系统）
> **学习目标**：掌握 UE 物理动画全栈技术——布娃娃、PhAT、Physical Animation Component、Chaos 布料

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-physics-asset.md](01-physics-asset.md) | Physics Asset（PhAT）完全指南 | ⭐⭐⭐ |
| [02-ragdoll-system.md](02-ragdoll-system.md) | 布娃娃系统：触发、恢复、过渡 | ⭐⭐⭐⭐ |
| [03-physical-animation.md](03-physical-animation.md) | Physical Animation Component（物理驱动动画）| ⭐⭐⭐⭐ |
| [04-hit-reaction.md](04-hit-reaction.md) | 受击反应（Hit Reaction）物理驱动 | ⭐⭐⭐⭐ |
| [05-chaos-cloth.md](05-chaos-cloth.md) | Chaos 布料模拟 | ⭐⭐⭐⭐ |
| [06-chaos-hair.md](06-chaos-hair.md) | Groom 毛发模拟 | ⭐⭐⭐ |
| [07-secondary-motion.md](07-secondary-motion.md) | 次级运动（Secondary Motion）| ⭐⭐⭐ |
| [08-rigid-body-anim.md](08-rigid-body-anim.md) | Rigid Body 动画节点 | ⭐⭐⭐⭐ |
| [09-physics-blend.md](09-physics-blend.md) | 物理与动画的混合过渡 | ⭐⭐⭐⭐ |
| [10-physics-optimization.md](10-physics-optimization.md) | 物理动画性能优化 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_ragdoll_controller.cpp](code/01_ragdoll_controller.cpp) | 完整布娃娃控制器（含恢复过渡）|
| [code/02_hit_reaction.cpp](code/02_hit_reaction.cpp) | 物理受击反应系统 |
| [code/03_physical_anim_setup.cpp](code/03_physical_anim_setup.cpp) | Physical Animation Component 配置 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/blogs.md](references/blogs.md) | 社区博客 |
| [references/videos.md](references/videos.md) | 视频教程 |

---

## 本章预计学习时间

- 阅读文章：5~7 小时
- 动手练习：4~6 小时
- **合计**：约 2~3 天
