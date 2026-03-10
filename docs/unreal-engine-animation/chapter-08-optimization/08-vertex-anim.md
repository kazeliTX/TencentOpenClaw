# 8.8 Vertex Animation Texture（顶点动画烘焙）

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、VAT 的适用场景

```
Vertex Animation Texture（VAT）：
  将骨骼动画"烘焙"为顶点位移纹理
  运行时通过材质和顶点着色器驱动动画
  完全绕过骨骼动画系统（零 AnimInstance/骨骼计算）

适用场景：
  ✅ 大量相同角色（人群、僵尸群、植物群）
  ✅ 不需要交互的背景角色（距离 50m+）
  ✅ 静态环境中的装饰性动画（旗帜、植物摇摆）
  ✅ 移动平台（骨骼动画太慢的场景）

不适用：
  ❌ 需要交互（受击、对话）的角色
  ❌ 需要动态混合的动画
  ❌ 精度要求高的近景角色
```

---

## 二、VAT 工作流

```
步骤 1：在 DCC 工具中烘焙 VAT
  推荐工具：Houdini VAT 插件（最强大）
  或 Blender Vertex Animation Texture 插件
  
  输出文件：
    PositionMap.tga    ← 顶点位置偏移纹理（RGB = XYZ 偏移）
    NormalMap.tga      ← 顶点法线纹理
    
步骤 2：在 UE 中设置材质
  导入 PositionMap 和 NormalMap
  创建材质：
    WorldPositionOffset = SampleTexture(PositionMap, UV2) × 位移缩放
    Normal              = SampleTexture(NormalMap, UV2)
    
    UV2 = float2(Time × AnimSpeed / TotalFrames, VertexID / VertexCount)
    → 每帧采样不同行的纹理 = 播放不同帧的动画

步骤 3：静态 Mesh + VAT 材质
  将 SkeletalMesh 转换为 StaticMesh（烘焙绑定姿势）
  应用 VAT 材质
  → 用最简单的 StaticMesh 播放复杂骨骼动画
```

---

## 三、VAT 性能数据

```
对比（场景中 500 个行走 NPC）：

方案              CPU 开销      GPU 开销    视觉质量
───────────────────────────────────────────────────
骨骼动画(LOD0)   ~250ms         低          高
骨骼动画(LOD2)   ~30ms          低          中
VAT              ~0ms            中          低-中
VAT (GPU 实例化) ~0ms            极低        低-中

VAT + GPU 实例化（Instanced Static Mesh）：
  500 个相同 NPC：单次 Draw Call
  CPU 开销几乎为零
  GPU 开销取决于顶点数（通常极低）
  → 处理千人规模人群的标准方案
```

---

## 四、UE 官方 VAT 插件

```
UE 提供官方工具：Vertex Animation Tool（Python 脚本）

位置：
  Engine/Extras/ScriptPlugin/Python/VertexAnimationTool

支持模式：
  Soft Body：软体动画（布料等）
  Rigid Body：刚体动画
  Skeletal：骨骼动画烘焙（最常用）
  Morph Target：变形目标烘焙
```

---

## 五、延伸阅读

- 🔗 [Vertex Animation Tool 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/vertex-animation-tool-in-unreal-engine)
- 🔗 [VAT 人群实战教程](https://dev.epicgames.com/community/learning/tutorials)
