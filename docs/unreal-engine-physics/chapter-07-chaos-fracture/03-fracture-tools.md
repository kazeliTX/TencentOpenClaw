# 7.3 Fracture 工具（Voronoi/平面/层级）

> **难度**：⭐⭐⭐⭐☆

## 一、Fracture 工具面板

```
启用插件：Edit → Plugins → Chaos Fracture Editor ✓
打开方式：模式面板 → Fracture Mode（斧头图标）

主要切割工具：

Voronoi（泰森多边形）：
  最常用，适合自然破碎（岩石/混凝土/陶罐）
  参数：
    Site Count：碎片数量（5-50 为常用范围）
    Random Seed：随机种子（固定破碎形状）
    ForestVoronoi：更均匀分布
    GridVoronoi：更规则（砖块感）
    RadialVoronoi：从中心向外辐射（爆炸感）

Planar（平面切割）：
  用平面切割，适合玻璃/晶体/几何建筑
  参数：
    Normal：切割面法线
    Grid X/Y：X/Y 方向切割数量
    Grid Spacing：切割间距（cm）

Slice（切片）：
  沿某个轴均匀切片
  适合面包/蛋糕/肌肉（层状结构）

Brick（砖块）：
  模拟砌砖破碎
  适合砖墙/石墙

Cluster（层级分组）：
  手动分组碎片，定义层级
```

## 二、层级破碎工作流

```
正确的层级破碎流程：

Step 1：粗碎（Level 0）
  Voronoi，Site Count = 4~6
  → 大块（整体崩裂的第一级）

Step 2：选中所有 Level 0 碎片
  Fracture 工具 → 再次 Voronoi，Site Count = 8~15
  → 在 Level 0 碎片内部细碎（Level 1）

Step 3：（可选）Level 2
  选中最小碎片 → Voronoi，Site Count = 20~50
  → 最细粒度（碎石渣）

Step 4：设置 Damage Threshold
  GC Details → Damage Threshold:
    [2000, 500, 100]  ← 层级越深越容易碎

实践建议：
  墙体：3 层，总碎片 < 200 个
  小道具（花盆）：1-2 层，总碎片 < 30 个
  大型建筑倒塌：4 层，总碎片 < 500 个（需要 LOD 控制）
```

## 三、Fracture 视图与选择

```
Fracture 模式下的视图按钮：
  Explode View（爆炸视图）：分开查看所有碎片
  Level View：按层级着色显示
  State View：显示哪些碎片固定/激活/睡眠

选择操作：
  点击 = 选择单个碎片
  Ctrl+点击 = 多选
  Shift+点击 = 层级选择（连同子级）
  Lasso：套索选择
```

## 四、延伸阅读

- 📄 [7.4 Field System](./04-field-system.md)
- 📄 [7.5 破碎阈值](./05-break-threshold.md)
