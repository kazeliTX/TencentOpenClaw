# 第十一章：MetaHuman 动画集成

> **前置要求**：已完成第三章（AnimBlueprint）、第十章（Control Rig）
> **学习目标**：掌握 MetaHuman 角色的动画系统，包括面部动画、Live Link、动作捕捉集成与性能优化

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-metahuman-overview.md](01-metahuman-overview.md) | MetaHuman 架构总览 | ⭐⭐⭐ |
| [02-body-animation.md](02-body-animation.md) | 身体动画系统 | ⭐⭐⭐⭐ |
| [03-face-animation.md](03-face-animation.md) | 面部动画系统（ARKit/FACS）| ⭐⭐⭐⭐ |
| [04-live-link.md](04-live-link.md) | Live Link 实时动作捕捉 | ⭐⭐⭐⭐ |
| [05-arkit-face.md](05-arkit-face.md) | ARKit 面部捕捉（iPhone）| ⭐⭐⭐⭐ |
| [06-control-rig-face.md](06-control-rig-face.md) | Control Rig 面部骨骼控制 | ⭐⭐⭐⭐⭐ |
| [07-anim-retarget-mh.md](07-anim-retarget-mh.md) | MetaHuman 动画重定向 | ⭐⭐⭐⭐ |
| [08-performance-lod.md](08-performance-lod.md) | MetaHuman 性能与 LOD | ⭐⭐⭐⭐ |
| [09-custom-character.md](09-custom-character.md) | 自定义角色接入 MetaHuman 系统 | ⭐⭐⭐⭐⭐ |
| [10-pipeline-guide.md](10-pipeline-guide.md) | MetaHuman 完整制作流程指南 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_livelink_controller.cpp](code/01_livelink_controller.cpp) | Live Link 数据接收与处理 |
| [code/02_face_anim_driver.cpp](code/02_face_anim_driver.cpp) | 面部动画程序化驱动 |
| [code/03_mh_anim_instance.cpp](code/03_mh_anim_instance.cpp) | MetaHuman AnimInstance 扩展 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/tools.md](references/tools.md) | 相关工具 |
| [references/tutorials.md](references/tutorials.md) | 教程资源 |
