# 第九章：网络同步动画

> **前置要求**：已完成第三章（AnimBlueprint）、第四章（混合过渡）
> **学习目标**：掌握多人联机游戏中角色动画的网络同步方案，从变量复制到自定义网络动画系统

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-network-basics.md](01-network-basics.md) | 网络架构基础与动画同步原理 | ⭐⭐⭐ |
| [02-movement-replication.md](02-movement-replication.md) | 移动同步与 CharacterMovement | ⭐⭐⭐⭐ |
| [03-anim-variable-replication.md](03-anim-variable-replication.md) | AnimInstance 变量复制 | ⭐⭐⭐⭐ |
| [04-montage-replication.md](04-montage-replication.md) | Montage 网络同步 | ⭐⭐⭐⭐ |
| [05-predict-correct.md](05-predict-correct.md) | 客户端预测与服务端校正 | ⭐⭐⭐⭐⭐ |
| [06-rootmotion-network.md](06-rootmotion-network.md) | Root Motion 网络同步 | ⭐⭐⭐⭐⭐ |
| [07-simulated-proxy.md](07-simulated-proxy.md) | SimulatedProxy 动画优化 | ⭐⭐⭐⭐ |
| [08-lag-compensation.md](08-lag-compensation.md) | 延迟补偿与动画 | ⭐⭐⭐⭐⭐ |
| [09-animation-notify-network.md](09-animation-notify-network.md) | AnimNotify 网络触发 | ⭐⭐⭐⭐ |
| [10-network-debug.md](10-network-debug.md) | 网络动画调试指南 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_replicated_anim_instance.cpp](code/01_replicated_anim_instance.cpp) | 带网络同步的 AnimInstance |
| [code/02_montage_replication.cpp](code/02_montage_replication.cpp) | Montage 多播同步系统 |
| [code/03_network_movement_anim.cpp](code/03_network_movement_anim.cpp) | 网络移动状态驱动动画 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/articles.md](references/articles.md) | 技术文章 |
| [references/videos.md](references/videos.md) | 视频教程 |

---

## 本章预计学习时间

- 阅读文章：5~6 小时
- 实践：4~6 小时
- **合计**：约 2 天
