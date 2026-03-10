# 8.9 大规模人群动画方案

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、大规模人群面临的挑战

```
人群规模 vs 方案选择：

规模         可行方案                      典型场景
──────────────────────────────────────────────────────────
< 20 人      完整骨骼动画（正常方案）      近处 NPC
20~100 人    骨骼动画 + LOD + Budget      中等规模战斗场景
100~500 人   骨骼动画(LOD2+) + VAT 混合   大型战场背景
500~5000 人  VAT + GPU Instancing         体育场人群
> 5000 人    粒子/纯 GPU 方案             演唱会/史诗场景
```

---

## 二、Crowd Manager 方案（UE 内置）

```
UE 内置 Crowd Manager（基于 RVO 导航避让）：
  适合 AI 驱动的人群导航
  每个 Agent 独立寻路，自动避让
  
  动画方面：结合 LOD + Budget Allocator
  → 近处：完整骨骼动画
  → 远处：VAT 或 Copy Pose

启用：
  Edit → Project Settings → Navigation → Crowd Manager: ✅
  Character → Use Client-Side Navigation: ✅

性能：
  300 个 Crowd Agent（AI 导航）：~5~15ms/帧（CPU）
  主要瓶颈在 RVO 避让计算，不是动画
```

---

## 三、Instanced Skeletal Mesh（ISM）+ 动画

```
UE5 的 Instanced Skeletal Mesh Component 支持：
  将多个相同骨骼 Mesh 合并为一次 Draw Call
  每个实例可以有独立的动画状态（动画帧偏移）

使用方式：
  用 AnimToTexture 插件将骨骼动画烘焙为纹理
  然后通过 Instanced Static Mesh 或 Niagara 大量复制

AnimToTexture 插件（UE5 官方）：
  自动将动画序列烘焙为骨骼变换纹理
  + 提供对应的 Material Function
  → 静态 Mesh 实例化 + 材质驱动动画
```

---

## 四、Niagara 人群方案（GPU 粒子人群）

```
最极端的大规模方案：用 Niagara 粒子系统模拟人群

每个 Niagara 粒子 = 一个人群成员
在 GPU 上计算每个粒子的位置和动画帧
使用 VAT 材质渲染每个粒子的 Mesh

能力：
  支持 10000+ 个人群成员 @ 60fps
  但每个成员几乎没有 AI（只有简单运动）
  适合：体育场观众、游行人群、战役远景人群

配置（概要）：
  1. 创建 Niagara 系统
  2. Spawn 粒子（每粒子代表一个人）
  3. 粒子更新：简单行走逻辑（速度 + 目标位置）
  4. 渲染器：Mesh Renderer → VAT 人物 Mesh
  5. 每粒子的 AnimFrame 变量驱动 VAT 材质
```

---

## 五、混合方案（推荐实际项目）

```
分层人群方案（LOD 思想应用到人群）：

近景（< 20m）：
  完整骨骼动画 + AnimBP
  支持受击、对话、AI 响应

中景（20~80m）：
  骨骼动画 LOD2 + 降频（15fps）
  简单 AI（导航 + 基础状态）

远景（80m+）：
  AnimToTexture + Instanced Static Mesh
  纯 GPU 驱动，CPU 零开销
  不支持 AI 响应（只是视觉装饰）

切换：
  当角色进入近景范围时，从 ISM 切换到 SkeletalMesh
  距离 < 20m：Spawn 完整 NPC，销毁对应 ISM 实例
  角色离开后：反向切换回 ISM
```

---

## 六、延伸阅读

- 📄 [代码示例：人群管理器](./code/03_crowd_manager.cpp)
- 🔗 [AnimToTexture 插件文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/anim-to-texture-in-unreal-engine)
- 🔗 [Niagara Mesh Renderer](https://dev.epicgames.com/documentation/en-us/unreal-engine/niagara-mesh-renderer-in-unreal-engine)
