# 第四章：动画混合与过渡

> **前置要求**：已完成第三章（AnimBP 与状态机）
> **学习目标**：掌握 Blend Space、Montage、Aim Offset、Additive 动画等 UE 核心混合技术

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-blend-space.md](01-blend-space.md) | Blend Space 完全指南 | ⭐⭐⭐ |
| [02-blend-space-1d.md](02-blend-space-1d.md) | 1D Blend Space 与 Aim Offset | ⭐⭐ |
| [03-aim-offset.md](03-aim-offset.md) | Aim Offset 深度解析 | ⭐⭐⭐ |
| [04-animation-montage.md](04-animation-montage.md) | Animation Montage 完全指南 | ⭐⭐⭐⭐ |
| [05-montage-slots.md](05-montage-slots.md) | Montage Slot 与混合层 | ⭐⭐⭐ |
| [06-additive-animation.md](06-additive-animation.md) | Additive 叠加动画 | ⭐⭐⭐ |
| [07-pose-snapshots.md](07-pose-snapshots.md) | Pose Snapshot 与 Pose Asset | ⭐⭐⭐ |
| [08-sync-groups.md](08-sync-groups.md) | Sync Groups 动画同步 | ⭐⭐⭐ |
| [09-motion-warping.md](09-motion-warping.md) | Motion Warping 根运动对齐 | ⭐⭐⭐⭐ |
| [10-blend-optimization.md](10-blend-optimization.md) | 混合系统性能优化 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_montage_manager.cpp](code/01_montage_manager.cpp) | Montage 播放管理器 |
| [code/02_motion_warping.cpp](code/02_motion_warping.cpp) | Motion Warping 目标设置 |
| [code/03_pose_snapshot.cpp](code/03_pose_snapshot.cpp) | Pose Snapshot 捕获与应用 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/blogs.md](references/blogs.md) | 社区博客 |
| [references/videos.md](references/videos.md) | 视频教程 |

---

## 本章预计学习时间

- 阅读文章：4~6 小时
- 动手练习：4~6 小时
- **合计**：约 2 天
