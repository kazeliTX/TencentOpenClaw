# 第八章：动画优化与 LOD

> **前置要求**：已完成前七章
> **学习目标**：掌握 UE5 动画系统的全套性能优化手段，从 LOD 策略到 Animation Budget Allocator

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-optimization-overview.md](01-optimization-overview.md) | 动画优化总览与性能分析工具 | ⭐⭐⭐ |
| [02-skeletal-mesh-lod.md](02-skeletal-mesh-lod.md) | Skeletal Mesh LOD 配置 | ⭐⭐⭐ |
| [03-anim-lod.md](03-anim-lod.md) | 动画系统 LOD（AnimBP LOD）| ⭐⭐⭐⭐ |
| [04-animation-budget.md](04-animation-budget.md) | Animation Budget Allocator | ⭐⭐⭐⭐ |
| [05-update-rate.md](05-update-rate.md) | 动画更新频率控制 | ⭐⭐⭐⭐ |
| [06-fast-path.md](06-fast-path.md) | Animation Fast Path 与线程优化 | ⭐⭐⭐⭐ |
| [07-npcanim-pool.md](07-npcanim-pool.md) | NPC 动画池与实例复用 | ⭐⭐⭐⭐ |
| [08-vertex-anim.md](08-vertex-anim.md) | Vertex Animation（顶点动画）| ⭐⭐⭐ |
| [09-crowd-system.md](09-crowd-system.md) | 大规模人群动画方案 | ⭐⭐⭐⭐ |
| [10-profiling-guide.md](10-profiling-guide.md) | 动画性能分析实战指南 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_anim_budget_setup.cpp](code/01_anim_budget_setup.cpp) | Animation Budget Allocator 配置 |
| [code/02_update_rate_controller.cpp](code/02_update_rate_controller.cpp) | 动画更新频率动态控制 |
| [code/03_crowd_manager.cpp](code/03_crowd_manager.cpp) | 简易人群动画管理器 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/talks.md](references/talks.md) | GDC/Epic 演讲 |
| [references/tools.md](references/tools.md) | 性能分析工具 |

---

## 本章预计学习时间

- 阅读文章：4~5 小时
- 实践优化：3~5 小时
- **合计**：约 1~2 天
