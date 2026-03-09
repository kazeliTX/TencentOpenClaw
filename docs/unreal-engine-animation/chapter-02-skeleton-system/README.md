# 第二章：骨骼系统深度解析

> **前置要求**：已完成第一章基础概念学习
> **学习目标**：深入理解 UE 骨骼系统的内部机制，掌握骨骼树操作、蒙皮算法、物理资产配置及运行时骨骼控制

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-bone-hierarchy.md](01-bone-hierarchy.md) | 骨骼层级深度解析 | ⭐⭐⭐ |
| [02-reference-pose.md](02-reference-pose.md) | 参考姿势与绑定矩阵 | ⭐⭐⭐ |
| [03-skinning-algorithms.md](03-skinning-algorithms.md) | 蒙皮算法：LBS vs DQS | ⭐⭐⭐⭐ |
| [04-skin-weights.md](04-skin-weights.md) | 蒙皮权重原理与优化 | ⭐⭐⭐ |
| [05-virtual-bones.md](05-virtual-bones.md) | 虚拟骨骼与辅助骨骼 | ⭐⭐ |
| [06-physics-asset.md](06-physics-asset.md) | 物理资产（PhAT）完全指南 | ⭐⭐⭐ |
| [07-ragdoll-system.md](07-ragdoll-system.md) | 布娃娃系统实现 | ⭐⭐⭐ |
| [08-bone-modification.md](08-bone-modification.md) | 运行时骨骼变换修改 | ⭐⭐⭐⭐ |
| [09-cloth-simulation.md](09-cloth-simulation.md) | Chaos 布料模拟 | ⭐⭐⭐ |
| [10-skeleton-optimization.md](10-skeleton-optimization.md) | 骨骼系统性能优化 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_bone_modifier.cpp](code/01_bone_modifier.cpp) | 运行时骨骼变换控制器 |
| [code/02_physics_anim.cpp](code/02_physics_anim.cpp) | 物理动画混合示例 |
| [code/03_ragdoll_controller.cpp](code/03_ragdoll_controller.cpp) | 布娃娃启停控制 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/skinning-papers.md](references/skinning-papers.md) | 蒙皮算法学术论文 |
| [references/physics-resources.md](references/physics-resources.md) | 物理模拟资源 |
| [references/optimization-guides.md](references/optimization-guides.md) | 性能优化指南 |

---

## 学习目标

完成本章后，你将能够：

1. 解释 UE 内部骨骼层级数据结构（`FReferenceSkeleton`）的工作原理
2. 理解蒙皮的数学本质，区分 LBS 和 DQS 的优缺点
3. 配置合理的物理资产（PhysicsAsset），实现真实布娃娃效果
4. 在运行时通过 C++ 直接修改骨骼变换
5. 实现动画与物理的平滑混合（Physical Animation Component）
6. 配置 Chaos 布料模拟
7. 掌握骨骼系统的关键性能优化手段

---

## 本章预计学习时间

- 阅读文章：4~6 小时
- 动手练习：3~5 小时
- **合计**：约 1.5 天
