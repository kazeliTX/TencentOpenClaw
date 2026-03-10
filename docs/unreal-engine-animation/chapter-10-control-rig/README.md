# 第十章：Control Rig 与程序化动画

> **前置要求**：已完成第三章（AnimBlueprint）、第五章（IK）
> **学习目标**：掌握 Control Rig 的创建与使用，实现程序化、可编辑的骨骼控制系统

---

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-control-rig-intro.md](01-control-rig-intro.md) | Control Rig 概述与架构 | ⭐⭐⭐ |
| [02-rig-hierarchy.md](02-rig-hierarchy.md) | Rig 层级：骨骼、控制器、空间 | ⭐⭐⭐⭐ |
| [03-rig-graph.md](03-rig-graph.md) | Rig Graph（可视化编程）| ⭐⭐⭐⭐ |
| [04-forward-solve.md](04-forward-solve.md) | Forward Solve（FK 正向求解）| ⭐⭐⭐⭐ |
| [05-backwards-solve.md](05-backwards-solve.md) | Backwards Solve（动画重定向）| ⭐⭐⭐⭐ |
| [06-ik-nodes.md](06-ik-nodes.md) | Control Rig IK 节点 | ⭐⭐⭐⭐⭐ |
| [07-procedural-anim.md](07-procedural-anim.md) | 程序化动画（呼吸/眨眼/IK）| ⭐⭐⭐⭐ |
| [08-anim-retarget.md](08-anim-retarget.md) | IK Retargeter 动画重定向 | ⭐⭐⭐⭐ |
| [09-control-rig-sequencer.md](09-control-rig-sequencer.md) | Control Rig + Sequencer 动画制作 | ⭐⭐⭐⭐ |
| [10-python-scripting.md](10-python-scripting.md) | Python 驱动 Control Rig | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_control_rig_cpp.cpp](code/01_control_rig_cpp.cpp) | C++ 调用 Control Rig |
| [code/02_procedural_spine.cpp](code/02_procedural_spine.cpp) | 程序化脊椎弯曲 |
| [code/03_ik_retarget_helper.cpp](code/03_ik_retarget_helper.cpp) | IK 重定向辅助工具 |

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/tutorials.md](references/tutorials.md) | 教程 |
| [references/papers.md](references/papers.md) | 相关论文 |
