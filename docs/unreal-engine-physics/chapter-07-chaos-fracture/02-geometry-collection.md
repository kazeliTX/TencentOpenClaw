# 7.2 GeometryCollection 资产详解

> **难度**：⭐⭐⭐⭐☆

## 一、创建 GeometryCollection

```
方式一：从 StaticMesh 创建
  内容浏览器 → 右键 StaticMesh → Create → Geometry Collection
  → 生成 .geometrycollection 文件
  → 原 SM 保留（GC 是独立资产）

方式二：在 Fracture 工具中创建
  选中场景中的 StaticMesh Actor
  工具栏 → Fracture（需启用 Fracture Editor 插件）
  → 自动转为 GeometryCollectionActor

GeometryCollection 属性（Details）：
  Damage Model：
    Single → 一次碰撞即破碎（适合玻璃）
    Accumulated → 累积伤害破碎（适合混凝土）
    
  Mass：
    Cluster Mass：整体质量（kg）
    
  Damage Threshold：
    破碎所需的最小冲击力（N）
    数组，每个元素对应一个层级
    [500, 200, 50] → Level 0 需要 500N，Level 1 需要 200N，Level 2 需要 50N
```

## 二、Cluster 配置

```
Cluster Union Mode：
  None：每个碎片独立物理
  Union：碎片先作为整体运动，破碎后分开
  
Breaking Mode：
  No Damage → 不会破碎（装饰用）
  Allow Damage → 允许破碎（默认）
  
Cluster Level 设置：
  Max Cluster Level：最大层级深度（越大越细碎）
  Cluster Count：每层的碎片数量目标
  
推荐层级设计：
  玻璃：2-3 层，大碎片 + 小碎片 + 玻璃渣
  混凝土墙：2 层，大块 + 碎石
  陶罐：1 层（直接碎）
  金属结构：3-4 层（大块→中块→碎片→金属屑）
```

## 三、Proximity Mode（连接检测）

```
破碎时碎片间如何检测连接关系：

Convex Hull（默认）：
  检测每对碎片的凸包是否接近
  精确但计算量大
  
Bounding Box：
  用 AABB 检测接近度
  性能好但不精确（可能有假连接）
  
Sphere：
  用包围球检测
  最快，适合大量小碎片

Connection Distance：
  连接距离阈值（cm）
  越大 = 越多碎片相互连接 = 破碎后牵连更多
  
Contact（接触面积）：
  连接强度与接触面积成正比
  大接触面 = 强连接（需要更大力才能分开）
  小接触面 = 弱连接（容易脆断）
```

## 四、延伸阅读

- 📄 [7.3 Fracture 工具](./03-fracture-tools.md)
