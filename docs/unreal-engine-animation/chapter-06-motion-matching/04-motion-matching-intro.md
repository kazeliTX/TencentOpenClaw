# 6.4 Motion Matching 概念与原理

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、传统状态机的局限

```
传统状态机驱动动画的问题：

问题 1：状态爆炸
  Walk → Run → Sprint → Crouch → CrouchWalk → ...
  每增加一个状态，过渡边数量指数级增长
  AAA 游戏状态机可能有数百个状态
  维护成本极高

问题 2：硬切换不自然
  从"向左跑"切换到"向右跑"：
  必须经过过渡动画（0.2~0.3s）
  如果来回快速切换 → 动画僵硬

问题 3：动画数量 vs 质量的平衡
  精细过渡需要大量"过渡动画"
  即便如此，边缘情况仍然生硬
  
问题 4：动画师负担重
  手动制作所有状态和过渡
  一个角色可能需要 200+ 个动画序列
```

---

## 二、Motion Matching 的核心思想

```
Motion Matching 的革命性理念：
  不用手动设计状态机
  让算法自动从动画数据库中找到"最适合当前角色状态"的动画帧

核心问题：
  给定角色当前状态（位置、速度、姿势、未来轨迹预测）
  从海量动画帧中找到"最匹配"的那一帧
  直接跳到那一帧开始播放
  
关键优势：
  过渡自然（直接接最相似帧，无需手动过渡动画）
  只需录制"干净"的原始动画，无需手动过渡
  角色运动对输入的响应更自然
```

---

## 三、Motion Matching 的技术组成

```
Motion Matching 系统三大核心组件：

1. Pose Search Database（姿势搜索数据库）
   包含所有待搜索的动画帧
   每帧存储"特征向量"（Pose Features）
   特征包括：骨骼位置/速度、轨迹点等
   
2. Pose Features（姿势特征）
   描述某一帧状态的向量
   通常包含：
   • 当前姿势特征（关键骨骼的位置/速度）
   • 轨迹特征（过去 N 帧/未来 N 帧的角色轨迹）
   
3. Pose Matching（匹配搜索）
   查询特征：角色当前状态 → 实时生成查询向量
   在数据库中找到特征距离最小的帧（最近邻搜索）
   跳转播放该帧
   
   匹配公式：
   Cost = Σ weight_i × (feature_i_query - feature_i_database)²
   → 找到 Cost 最小的数据库帧
```

---

## 四、Motion Matching 的历史

```
Motion Matching 发展历程：

2015：Simon Clavet（Ubisoft）发表 GDC 演讲
      "Motion Matching, The Road to Next Gen Animation"
      在《荣耀战魂》中首次商业应用

2016~2020：游戏工业界广泛研究和实现
      《最后生还者 Part II》（Naughty Dog）
      《控制》（Remedy）

2022：Ubisoft 的 Kinetix、EA 等大厂广泛应用

2023：Epic 在 UE5.4 中发布官方 Motion Matching 插件
      集成到 Pose Search 插件中

关键论文：
  Daniel Holden et al., "Phase-Functioned Neural Networks for Character Control" (SIGGRAPH 2017)
  Lucas Kovar et al., "Motion Graphs" (SIGGRAPH 2002)
```

---

## 五、UE5 Motion Matching 插件

```
UE5 中使用 Motion Matching 需要启用插件：

Edit → Plugins → Animation → 搜索并启用：
  • Pose Search（必须）
  • Motion Trajectory（必须，轨迹预测）
  • Animation Warping（推荐，配合使用）

启用后新增的资产类型：
  Pose Search Database     ← 动画数据库
  Pose Search Schema       ← 特征配置（描述哪些骨骼/轨迹）
  Motion Matching Config   ← 搜索参数配置
```

---

## 六、延伸阅读

- 📄 [6.5 Pose Search 姿势搜索](./05-pose-search.md)
- 📄 [6.6 轨迹预测](./06-trajectory-prediction.md)
- 🔗 [Motion Matching 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-matching-in-unreal-engine)
- 🔗 [Simon Clavet GDC 2015 演讲](https://www.gdcvault.com/play/1022030/Motion-Matching-and-The-Road)
- 🔗 [Motion Matching 知乎深度解析](https://zhuanlan.zhihu.com/p/1927668362969880273)
