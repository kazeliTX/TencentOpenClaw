# 1.10 本章学习路径与推荐资源

> 按照这条路径学习，效率最高

---

## 推荐学习顺序

```
Week 1：理解概念
  Day 1-2: 01-animation-system-overview.md（架构总览）
  Day 3-4: 02-skeleton-asset.md（骨骼资产）
  Day 5-6: 04-animation-asset-types.md（资产类型）
  Day 7:   05-coordinate-systems.md（坐标系，重要！）

Week 2：动手实践
  Day 1-2: 06-animation-pipeline.md（管线流程）
           → 实操：导入一个 FBX，配置 AnimBP 播放动画
  Day 3-4: 07-core-components.md（核心组件）
           → 实操：C++ 代码控制 AnimInstance
  Day 5-6: 08-ue4-vs-ue5.md（版本差异）
  Day 7:   复习 + 代码练习（见 code/ 目录）
```

---

## 官方文档（必读）

| 文档 | URL | 优先级 |
|------|-----|--------|
| 动画系统概述（中文）| https://dev.epicgames.com/documentation/zh-cn/unreal-engine/animation-system-overview | ⭐⭐⭐⭐⭐ |
| Skeletal Mesh 动画系统 | https://dev.epicgames.com/documentation/en-us/unreal-engine/skeletal-mesh-animation-system-in-unreal-engine | ⭐⭐⭐⭐⭐ |
| 骨骼资产文档 | https://dev.epicgames.com/documentation/en-us/unreal-engine/skeletons-in-unreal-engine | ⭐⭐⭐⭐ |
| Animation Blueprint 文档 | https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprints-in-unreal-engine | ⭐⭐⭐⭐⭐ |
| Blend Space 文档 | https://dev.epicgames.com/documentation/en-us/unreal-engine/blend-spaces-in-unreal-engine | ⭐⭐⭐⭐ |
| Animation Montage 文档 | https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-montage-in-unreal-engine | ⭐⭐⭐⭐ |

---

## 推荐视频教程

| 视频 | 平台 | 说明 |
|------|------|------|
| UE5 Animation Blueprint Part 1: State Machines | YouTube（Ryan Laley）| 最佳入门视频之一 |
| UE5 实战指南：从零构建高级运动系统 | B 站 BV1BRPKzyEYL | 中文，含完整运动系统搭建 |
| Unreal Engine 5 Tutorial - Animation Blueprints | YouTube | Epic 官方教程系列 |

---

## 推荐博客/文章

| 文章 | 平台 | 说明 |
|------|------|------|
| UE5 Animation 基础概念总结 | 知乎 https://zhuanlan.zhihu.com/p/671487108 | 中文，概念清晰 |
| 剖析 UE 动画系统核心脉络 | ByteZoneX https://www.bytezonex.com/archives/pZ8eaZhE.html | 源码级分析 |
| 解析UE动画系统——核心实现 | SegmentFault https://segmentfault.com/a/1190000044591404 | 运行时原理 |
| UE5 我对 UAF 的理解（5.6）| 知乎 https://zhuanlan.zhihu.com/p/1940811608155726731 | 最新架构解读 |
| The Unreal Animation System（英文）| developers-heaven.net | 英文深度好文 |

---

## 第一章代码练习清单

完成本章学习后，请完成以下练习：

- [ ] **练习1**：创建一个 C++ Character，配置 AnimBP，实现 Idle/Walk/Run 状态机
- [ ] **练习2**：用 C++ 代码读取角色某根骨骼的 World Transform 并打印
- [ ] **练习3**：创建自定义 AnimNotify，在脚步动画时播放音效
- [ ] **练习4**：实现 Master Pose Component，让帽子/武器 Mesh 跟随角色骨骼
- [ ] **练习5**：使用 IK Retargeter 将 Mixamo 动画重定向到 UE5 Mannequin

所有练习的参考代码见 `code/` 目录。
