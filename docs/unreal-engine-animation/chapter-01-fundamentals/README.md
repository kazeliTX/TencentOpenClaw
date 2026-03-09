# 第一章：基础概念与架构

> 本章是整个 UE 骨骼动画系统的入门基础，建议所有人首先阅读。

## 📁 本章文件结构

```
chapter-01-fundamentals/
├── README.md                         ← 本文件（导读）
├── 01-animation-system-overview.md   ← 1.1 动画系统总览与架构
├── 02-skeleton-asset.md              ← 1.2 骨骼资产（Skeleton）详解
├── 03-skeletal-mesh.md               ← 1.3 骨骼网格体（Skeletal Mesh）
├── 04-animation-asset-types.md       ← 1.4 动画资产类型全览
├── 05-coordinate-systems.md          ← 1.5 骨骼空间与坐标系
├── 06-animation-pipeline.md          ← 1.6 动画管线（Pipeline）全流程
├── 07-core-components.md             ← 1.7 核心组件详解
├── 08-ue4-vs-ue5.md                  ← 1.8 UE4 vs UE5 动画系统差异
├── 09-glossary.md                    ← 1.9 术语速查表
├── 10-learning-path.md               ← 1.10 本章学习路径与资源
├── references/
│   ├── official-docs.md              ← 官方文档链接汇总
│   ├── blogs-and-articles.md         ← 推荐博客与文章
│   └── videos.md                     ← 推荐视频资源
└── code/
    ├── 01_basic_anim_instance.cpp    ← AnimInstance 基础示例
    ├── 02_skeleton_query.cpp         ← 骨骼查询工具函数
    └── 03_asset_loader.cpp           ← 动画资产运行时加载
```

## 🎯 本章学习目标

完成本章后，你应该能够：

- [ ] 理解 UE 动画系统的整体架构与分层设计
- [ ] 区分 Skeleton、Skeletal Mesh、AnimSequence 等核心资产
- [ ] 掌握 Local/Component/World 三种骨骼空间的概念
- [ ] 了解动画从 DCC 到引擎运行时的完整管线
- [ ] 能够配置基础的 SkeletalMeshComponent 并播放动画

## ⏱️ 预计学习时间

- 快速浏览：1 小时
- 深度学习：4-6 小时
- 含代码练习：8-10 小时
