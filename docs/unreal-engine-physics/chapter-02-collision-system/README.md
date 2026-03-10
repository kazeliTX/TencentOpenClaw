# 第二章：碰撞系统详解

> **前置要求**：完成第一章，理解刚体基础与物理场景
> **学习目标**：掌握 UE 碰撞系统的完整工作机制

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-collision-shapes.md](01-collision-shapes.md) | 碰撞体类型（Simple/Complex/多形状组合） | ⭐⭐⭐ |
| [02-collision-channels.md](02-collision-channels.md) | 碰撞通道（Channel）系统 | ⭐⭐⭐⭐ |
| [03-collision-response.md](03-collision-response.md) | 碰撞响应（Block/Overlap/Ignore）| ⭐⭐⭐⭐ |
| [04-collision-profiles.md](04-collision-profiles.md) | Collision Profile 设计与管理 | ⭐⭐⭐⭐ |
| [05-hit-events.md](05-hit-events.md) | Hit Event 详解与使用 | ⭐⭐⭐ |
| [06-overlap-events.md](06-overlap-events.md) | Overlap Event 详解与使用 | ⭐⭐⭐ |
| [07-trace-channels.md](07-trace-channels.md) | Trace Channel 与物理查询通道 | ⭐⭐⭐⭐ |
| [08-complex-collision.md](08-complex-collision.md) | 复杂碰撞（Complex Collision）与优化 | ⭐⭐⭐⭐ |
| [09-multi-body-collision.md](09-multi-body-collision.md) | 多碰撞体组合与骨骼碰撞 | ⭐⭐⭐⭐ |
| [10-collision-filtering.md](10-collision-filtering.md) | 碰撞过滤、忽略 Actor/Component | ⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_collision_setup.cpp](code/01_collision_setup.cpp) | 运行时碰撞通道/响应/Profile 设置 |
| [code/02_hit_overlap_events.cpp](code/02_hit_overlap_events.cpp) | Hit & Overlap 事件处理完整示例 |
| [code/03_custom_trace_channel.cpp](code/03_custom_trace_channel.cpp) | 自定义 Trace Channel 查询示例 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/papers.md](references/papers.md) | 相关论文 |
| [references/blogs.md](references/blogs.md) | 博客与文章 |
