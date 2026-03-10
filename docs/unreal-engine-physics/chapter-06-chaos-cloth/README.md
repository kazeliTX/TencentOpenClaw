# 第六章：Chaos 布料系统

> **前置要求**：第一章（Chaos 架构）、第三章（约束）
> **学习目标**：掌握 Chaos Cloth 配置、风场、参数调优与性能优化

## 本章文件目录

| 文件 | 主题 | 难度 |
|------|------|------|
| [01-chaos-cloth-overview.md](01-chaos-cloth-overview.md) | Chaos Cloth 总览与架构 | ⭐⭐⭐ |
| [02-cloth-asset-setup.md](02-cloth-asset-setup.md) | Cloth Asset 配置流程 | ⭐⭐⭐⭐ |
| [03-cloth-parameters.md](03-cloth-parameters.md) | 布料参数详解（刚度/阻尼/摩擦） | ⭐⭐⭐⭐⭐ |
| [04-wind-field.md](04-wind-field.md) | 风场配置（WindDirectionalSource）| ⭐⭐⭐⭐ |
| [05-cloth-collision.md](05-cloth-collision.md) | 布料碰撞（自碰撞/环境碰撞）| ⭐⭐⭐⭐⭐ |
| [06-painted-weights.md](06-painted-weights.md) | 权重绘制（固定/最大距离）| ⭐⭐⭐⭐ |
| [07-cloth-lod.md](07-cloth-lod.md) | 布料 LOD 与性能设置 | ⭐⭐⭐⭐ |
| [08-chaos-cloth-panel.md](08-chaos-cloth-panel.md) | Chaos Cloth 编辑器详解 | ⭐⭐⭐⭐ |
| [09-runtime-cloth-control.md](09-runtime-cloth-control.md) | 运行时布料控制（重置/参数切换）| ⭐⭐⭐⭐⭐ |
| [10-cloth-common-issues.md](10-cloth-common-issues.md) | 常见问题与调试指南 | ⭐⭐⭐⭐ |

### 代码示例
| 文件 | 说明 |
|------|------|
| [code/01_cloth_runtime_control.cpp](code/01_cloth_runtime_control.cpp) | 运行时布料参数修改 |
| [code/02_wind_system.cpp](code/02_wind_system.cpp) | 动态风场控制系统 |
| [code/03_cloth_tearing.cpp](code/03_cloth_tearing.cpp) | 布料撕裂效果（近似实现）|

### 参考资源
| 文件 | 说明 |
|------|------|
| [references/official-docs.md](references/official-docs.md) | 官方文档 |
| [references/papers.md](references/papers.md) | 相关论文 |
| [references/blogs.md](references/blogs.md) | 博客与文章 |
