# 7.5 Chaos 布料模拟

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、Chaos Cloth 概述

```
Chaos Cloth 是 UE5 的新一代布料模拟系统：
  取代了旧版的 Nvidia PhysX Cloth（NvCloth）
  基于 Chaos Physics 引擎
  支持自碰撞、角色碰撞、风场等

典型用途：
  角色斗篷/披风（最常见）
  裙摆
  旗帜/帷幕
  宽松衣物（袖子、下摆）
```

---

## 二、布料创建工作流（编辑器）

```
步骤 1：在 DCC（Maya/Blender）中准备布料网格
  布料区域应与角色 Mesh 分离（独立三角面）
  或作为同一 Mesh 的单独部分（Section）

步骤 2：在 UE 编辑器中配置布料
  打开 SkeletalMesh 编辑器
  顶部菜单 → Clothing → Create ClothingAsset from Section
  选择要模拟的 Mesh Section（如斗篷部分）
  
  生成：ClothingAsset（绑定到该 Section）

步骤 3：绘制布料权重（Cloth Paint）
  Paint Mode → Cloth Paint 面板
  绘制 Max Distance（最大移动距离）：
    0.0   = 完全固定（附着点，如肩部固定区域）
    1.0   = 完全自由（最大物理运动，如斗篷下摆）
  建议：肩部/固定边 = 0，越往下越接近 1
```

---

## 三、Chaos Cloth Config 参数

```
打开 Clothing Asset → Config 面板：

物理参数：
  Gravity Scale:        1.0     布料受重力影响（1=正常重力）
  Linear Drag:          0.01    空气阻力（越大越慢）
  Angular Drag:         0.01
  Linear Inertia Scale: 0.8     跟随角色运动的惯性（1=完全跟随，0=纯物理）
  
碰撞参数：
  Collision Thickness:  1.0     布料厚度（防穿插）
  Friction Coefficient: 0.2     与角色碰撞体的摩擦力
  
约束参数：
  Stretch Stiffness:    1.0     拉伸刚度（越高越不能被拉伸）
  Bend Stiffness:       0.5     弯曲刚度（越高越硬，不易弯折）
  
阻尼：
  Damping Coefficient:  0.01    运动阻尼（防止持续振荡）

风力响应：
  Wind Method: Accurate / Legacy
  Drag Coefficient:     0.02    风阻（影响风力响应速度）
  Lift Coefficient:     0.02    升力
```

---

## 四、角色碰撞体配置

```
确保角色 Physics Asset 中有足够的碰撞体：

主要碰撞体（与布料交互）：
  ✅ 躯干（Capsule）
  ✅ 双腿（Capsule）
  ✅ 上臂（Capsule）
  
配置布料碰撞通道：
  ClothingAsset → Collision Channel: 默认会与 Physics Asset 的 Body 碰撞
  确保 Physics Asset Body 的 Simulation Filter 包含 Cloth 通道

性能优化：
  减少与布料碰撞的 Body 数量
  手部/头部等小 Body 通常不需要与布料碰撞
  → 在 Physics Asset Body 中禁用 Cloth Collision
```

---

## 五、Chaos Cloth 性能

```
布料模拟开销（参考值，单角色斗篷约 500 顶点）：
  单角色：~0.5~2ms（取决于顶点数和迭代次数）
  
优化策略：
  减少布料顶点（< 500 顶点可保持流畅）
  LOD 降低布料模拟：LOD2+ 可禁用布料物理
    USkeletalMeshComponent::SetClothingSimulationFactory
  增大时间步（降低精度换性能）：
    Chaos Solver → SubStep Count: 1（默认 2）
```

---

## 六、延伸阅读

- 🔗 [Chaos Cloth 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/clothing-simulation-in-unreal-engine)
- 🔗 [布料模拟性能指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/clothing-tool-in-unreal-engine)
