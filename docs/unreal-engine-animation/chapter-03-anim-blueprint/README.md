# 第三章：AnimBP 与状态机

> **前置要求**：已完成第一章（基础概念）和第二章（骨骼系统）
> **学习目标**：深入掌握 Animation Blueprint 架构、State Machine 设计模式、线程模型与性能最佳实践

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-animblueprint-architecture.md](01-animblueprint-architecture.md) | AnimBP 架构与线程模型 | ⭐⭐⭐ |
| [02-event-graph.md](02-event-graph.md) | Event Graph 与变量更新 | ⭐⭐ |
| [03-anim-graph.md](03-anim-graph.md) | AnimGraph 节点系统 | ⭐⭐⭐ |
| [04-state-machine-basics.md](04-state-machine-basics.md) | 状态机基础与设计模式 | ⭐⭐⭐ |
| [05-state-machine-advanced.md](05-state-machine-advanced.md) | 状态机进阶：条件、别名、子状态机 | ⭐⭐⭐⭐ |
| [06-transition-rules.md](06-transition-rules.md) | 过渡规则与混合曲线 | ⭐⭐⭐ |
| [07-anim-instance-cpp.md](07-anim-instance-cpp.md) | C++ AnimInstance 完整实现 | ⭐⭐⭐⭐ |
| [08-linked-anim-graph.md](08-linked-anim-graph.md) | Linked AnimGraph 模块化设计 | ⭐⭐⭐⭐ |
| [09-notify-system.md](09-notify-system.md) | Animation Notify 系统 | ⭐⭐⭐ |
| [10-animblueprint-optimization.md](10-animblueprint-optimization.md) | AnimBP 性能优化实践 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_anim_instance.cpp](code/01_anim_instance.cpp) | 完整 C++ AnimInstance 实现 |
| [code/02_state_machine_data.cpp](code/02_state_machine_data.cpp) | 状态机数据结构设计 |
| [code/03_custom_notify.cpp](code/03_custom_notify.cpp) | 自定义 AnimNotify 实现 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档链接 |
| [references/blogs.md](references/blogs.md) | 社区博客与教程 |
| [references/videos.md](references/videos.md) | 视频教程资源 |

---

## 学习目标

完成本章后，你将能够：

1. 清晰描述 AnimBP 的双线程执行模型（Game Thread / Worker Thread）
2. 正确分离 Event Graph（逻辑读取）和 AnimGraph（骨骼计算）
3. 设计清晰、可维护的角色状态机层级
4. 用 C++ 实现高性能的 `UAnimInstance` 子类
5. 使用 Linked AnimGraph 实现模块化动画系统
6. 理解并正确使用各类 AnimNotify
7. 掌握 AnimBP 的常见性能陷阱和解决方案

---

## 本章预计学习时间

- 阅读文章：4~6 小时
- 动手练习：4~6 小时
- **合计**：约 2 天
