# 8.2 Skeletal Mesh LOD 配置

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、Skeletal Mesh LOD 的意义

```
骨骼 Mesh 的两个性能维度：
  顶点数：影响蒙皮计算（GPU）
  骨骼数：影响动画计算（CPU）

LOD 对骨骼数的影响尤为重要：
  LOD0：200 根骨骼（完整骨骼）
  LOD1：60 根骨骼（移除手指、脸部细节）
  LOD2：25 根骨骼（只保留主要骨骼）
  LOD3：10 根骨骼（极简，几乎静止）

动画计算量与骨骼数近似线性关系：
  200→60 骨骼：动画计算降低 ~70%
```

---

## 二、创建 Skeletal Mesh LOD

```
方法 1：自动生成 LOD（推荐入门）
  打开 SkeletalMesh 编辑器
  顶部 Details → LOD Settings → LOD Group → Character
  LOD Import → Generate LOD
  UE 自动减面，但需要手动调整骨骼 LOD

方法 2：DCC 工具手动制作（推荐高质量项目）
  Maya/Blender 中制作各 LOD Mesh
  导入时指定 LOD 编号（Import As LOD X）

方法 3：UE 自动减骨（推荐）
  SkeletalMesh → LOD1 → Bone Reduction Settings
  Bone Reduction Asset: 指定要移除的骨骼列表
  Remove Specific Bones:
    LOD1 移除：所有手指骨骼（10+ 根）
    LOD2 移除：所有脸部骨骼、脚趾、手腕扭转辅助骨骼
    LOD3 移除：所有非主干骨骼
```

---

## 三、LOD 屏幕尺寸阈值

```
SkeletalMesh 编辑器 → LOD Info：

LOD   Screen Size   说明
─────────────────────────────────────────
0     1.0           角色占满屏幕（极近距离）
1     0.3           中距离（通常 5~15m）
2     0.1           远距离（15~40m）
3     0.03          极远（> 40m）

实际像素对应关系（1080p 屏幕）：
  Screen Size 0.3 ≈ 角色高度占屏幕 30% ≈ 约 300 像素高
  Screen Size 0.1 ≈ 约 100 像素高
  Screen Size 0.03 ≈ 约 30 像素高（几乎看不清细节）

调整原则：
  近战 FPS 游戏：LOD 切换更激进（玩家不会注意远处细节）
  肩视角 TPS/RPG：需要更保守的 LOD（玩家能看清配件）
```

---

## 四、蒙皮权重 LOD 优化

```
高 LOD（LOD0）：4 根骨骼权重（精细蒙皮）
低 LOD（LOD2+）：可降为 1~2 根骨骼权重（快速蒙皮）

配置：
  SkeletalMesh → LOD2 → Sections → Max Bone Influences: 2
  （默认 4，降为 2 可提升蒙皮计算速度约 30%）

注意：
  Max Bone Influences 降低会导致关节处变形质量下降
  LOD2 以上通常可接受（屏幕上很小）
```

---

## 五、延伸阅读

- 📄 [8.3 动画系统 LOD](./03-anim-lod.md)
- 🔗 [Skeletal Mesh LOD 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/skeletal-mesh-level-of-detail-in-unreal-engine)
