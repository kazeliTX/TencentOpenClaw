# 第六章：根运动与 Motion Matching

> **前置要求**：已完成第三章（AnimBP）、第四章（混合系统）
> **学习目标**：掌握 Root Motion 的完整工作原理，以及 UE5.4+ 的 Motion Matching 系统

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-root-motion-basics.md](01-root-motion-basics.md) | Root Motion 基础原理 | ⭐⭐⭐ |
| [02-root-motion-config.md](02-root-motion-config.md) | Root Motion 配置与工作流 | ⭐⭐⭐ |
| [03-root-motion-network.md](03-root-motion-network.md) | Root Motion 网络同步 | ⭐⭐⭐⭐ |
| [04-motion-matching-intro.md](04-motion-matching-intro.md) | Motion Matching 概念与原理 | ⭐⭐⭐⭐ |
| [05-pose-search.md](05-pose-search.md) | Pose Search（姿势搜索）| ⭐⭐⭐⭐ |
| [06-trajectory-prediction.md](06-trajectory-prediction.md) | 轨迹预测与运动特征 | ⭐⭐⭐⭐ |
| [07-mm-database.md](07-mm-database.md) | Motion Matching 数据库构建 | ⭐⭐⭐⭐ |
| [08-mm-animation-graph.md](08-mm-animation-graph.md) | Motion Matching 在 AnimGraph 中的配置 | ⭐⭐⭐⭐ |
| [09-mm-vs-statemachine.md](09-mm-vs-statemachine.md) | Motion Matching vs 状态机对比与混合 | ⭐⭐⭐ |
| [10-mm-optimization.md](10-mm-optimization.md) | Motion Matching 性能优化 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_root_motion_controller.cpp](code/01_root_motion_controller.cpp) | Root Motion 网络同步控制器 |
| [code/02_trajectory_generator.cpp](code/02_trajectory_generator.cpp) | 角色轨迹预测生成器 |
| [code/03_motion_matching_config.cpp](code/03_motion_matching_config.cpp) | Motion Matching 数据库配置 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/papers.md](references/papers.md) | Motion Matching 原始论文 |
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/tutorials.md](references/tutorials.md) | 社区教程与视频 |

---

## 本章预计学习时间

- 阅读文章：6~8 小时
- 动手练习：6~10 小时（Motion Matching 需要大量动画资产）
- **合计**：约 3~4 天
