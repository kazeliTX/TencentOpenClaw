# 11.8 MetaHuman 性能与 LOD

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、MetaHuman 的性能成本

```
默认 MetaHuman（LOD 0）的开销估算（单角色）：

组件              多边形数    主要开销
──────────────────────────────────────────────────────
Body Mesh        约 20万 三角面   蒙皮（GPU）
Face Mesh        约 25万 三角面   BlendShape + 蒙皮
Groom（头发）    5000~10000 发丝  Strand 渲染（GPU 密集）
Eye Mesh         约 5万 三角面    半透明材质（GPU）
Teeth/Mouth      约 3万 三角面    内部 Mesh

总 GPU 开销（单 MetaHuman @ 1080p）：
  ~3~8ms GPU 时间（取决于硬件和分辨率）

面部 AnimBP 开销（CPU）：
  ~0.5~2ms（Control Rig + 52 BlendShape 求值）

结论：MetaHuman 适合 1~5 个主角角色，不适合大量 NPC
```

---

## 二、LOD 配置

```
MetaHuman 内置 LOD 系统：

Body LOD：
  LOD 0：完整 Mesh（25万面）
  LOD 1：约 10万面
  LOD 2：约 5万面
  LOD 3：约 2万面（远景）

Face LOD（关键）：
  LOD 0：完整 BlendShape（52 个）+ 骨骼控制
  LOD 1：简化 BlendShape（减少数量）
  LOD 2：禁用 BlendShape（纯骨骼驱动）
  LOD 3：禁用 Face Control Rig（极简）
  
  配置：Face AnimBP → LOD Settings → 各 LOD 级别的 Control Rig 配置

Groom LOD：
  LOD 0：完整发丝（5000+ 根）
  LOD 1：减少发丝（50%）
  LOD 2：Cards（面片发型，快 5~10x）
  LOD 3：关闭 Groom（使用 Mesh 发型）
```

---

## 三、性能优化策略

```
策略 1：Groom 降级（最大收益）
  Groom 是最贵的部分！
  LOD 1+ 立即切换为 Cards
  或完全用 Cards（牺牲少量质量，性能大幅提升）

策略 2：减少活跃 MetaHuman 数量
  同时在视野中的完整 MetaHuman < 3 个（推荐）
  远处 MetaHuman：强制 LOD 2+ 禁用 Groom 和大部分 BlendShape

策略 3：Face AnimBP 降频
  次要 NPC 的 Face AnimBP：降为 15fps 更新
  SetComponentTickInterval(Face, 1.f/15.f)

策略 4：Nanite（UE5.1+）
  Nanite 支持 Skeletal Mesh（实验性）
  启用后 Body/Face Mesh 多边形数不再是性能瓶颈
  主要瓶颈转移到 Groom 和 BlendShape

命令行开关：
  r.MetaHuman.EnableLOD 2     ← 强制所有 MetaHuman 使用 LOD 2
  r.Groom.Enable 0            ← 全局禁用 Groom（调试用）
```

---

## 四、延伸阅读

- 🔗 [MetaHuman 性能指南](https://dev.epicgames.com/documentation/en-us/metahuman/metahuman-performance-guide)
- 🔗 [Nanite for Skeletal Mesh](https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine)
