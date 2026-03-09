# 第五章：IK 系统

> **前置要求**：已完成第三章（AnimBP）和第四章（混合系统）
> **学习目标**：掌握 UE 全套 IK 技术——从基础双骨骼 IK 到全身 IK，以及脚步 IK 的工程实现

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-ik-fundamentals.md](01-ik-fundamentals.md) | IK 数学基础与 UE 中的 IK 框架 | ⭐⭐⭐ |
| [02-two-bone-ik.md](02-two-bone-ik.md) | Two-Bone IK 完全指南 | ⭐⭐⭐ |
| [03-fabrik.md](03-fabrik.md) | FABRIK 链式 IK | ⭐⭐⭐ |
| [04-foot-ik.md](04-foot-ik.md) | 脚步 IK 完整实现（地形适配）| ⭐⭐⭐⭐ |
| [05-hand-ik.md](05-hand-ik.md) | 手部 IK（武器握持 / 双手持枪）| ⭐⭐⭐ |
| [06-ik-rig.md](06-ik-rig.md) | IK Rig 编辑器完全指南 | ⭐⭐⭐⭐ |
| [07-full-body-ik.md](07-full-body-ik.md) | Full Body IK（FBIK）| ⭐⭐⭐⭐ |
| [08-ik-retargeter.md](08-ik-retargeter.md) | IK Retargeter 动画重定向 | ⭐⭐⭐ |
| [09-control-rig-ik.md](09-control-rig-ik.md) | Control Rig 中的 IK 求解器 | ⭐⭐⭐⭐ |
| [10-ik-optimization.md](10-ik-optimization.md) | IK 系统性能优化 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_foot_ik_component.cpp](code/01_foot_ik_component.cpp) | 完整脚步 IK 组件 |
| [code/02_hand_ik_controller.cpp](code/02_hand_ik_controller.cpp) | 手部双手持枪 IK |
| [code/03_ik_trace_system.cpp](code/03_ik_trace_system.cpp) | IK 地面检测系统 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/ik-papers.md](references/ik-papers.md) | IK 算法学术论文 |
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/tutorials.md](references/tutorials.md) | 教程资源 |

---

## 本章预计学习时间

- 阅读文章：5~7 小时
- 动手练习：5~8 小时（脚步 IK 实现较复杂）
- **合计**：约 2~3 天
