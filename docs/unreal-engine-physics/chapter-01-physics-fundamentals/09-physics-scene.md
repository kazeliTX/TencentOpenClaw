# 1.9 物理场景与世界设置

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、World Settings 中的物理参数

```
关卡编辑器 → 右上角 Settings → World Settings → Physics：

Enable World Gravity:           true
Global Gravity Scale:           1.0  ← 全局重力倍数

Default Gravity Z:
  Project Settings → Physics → Default Gravity Z: -980.0 cm/s²

Enable Chaos:                   true（UE5 默认）

Physical Collision Handler:
  碰撞响应处理器（可自定义）

Async Physics Frame Time:
  Fixed Time Step（如启用 Async Physics）

Override World Gravity:
  可以在特定关卡覆盖全局重力
  例：太空关卡 Global Gravity Scale = 0.0
```

---

## 二、Project Settings → Physics 关键参数

```
物理精度相关：
  
  Bounce Threshold Velocity（弹跳阈值速度）：
    默认：200.0 cm/s
    含义：碰撞相对速度低于此值时，不计算弹跳（直接静止）
    过小 → 物体一直弹跳（果冻感）
    过大 → 物体落地时突然"吸住"（不自然）
    建议：100~300 cm/s

  Max Angular Velocity（最大角速度）：
    默认：3600 deg/s（10 转/秒）
    限制物理刚体的最大旋转速度
    防止数值爆炸导致的无限旋转

  Max Depenetration Velocity（最大去穿透速度）：
    默认：100.0 cm/s
    当两个物体重叠时，分离速度上限
    过大 → 重叠时剧烈弹开
    过小 → 重叠很久才分开

摩擦与弹性（全局默认）：
  Default Friction:             0.7
  Default Restitution:          0.3
  Friction Combine Mode:        Average（Average/Min/Max/Multiply）
  Restitution Combine Mode:     Average

Sleep 阈值：
  Sleep Linear Velocity Threshold:  2.0 cm/s
  Sleep Angular Velocity Threshold: 0.05 rad/s
  Sleep Counter Threshold:          3（连续 N 帧低于阈值才 Sleep）
```

---

## 三、Chaos Solver 设置

```
Project Settings → Physics → Chaos Physics:

Solver Iterations（求解迭代次数）：
  Collision Pair Iterations:  8  ← 碰撞约束迭代
  Joint Pair Iterations:      4  ← 关节约束迭代
  Collision Margin Fraction:  0.01
  
  迭代次数越多：
    精度越高（物体不容易穿透/抖动）
    CPU 开销越大
    建议：高质量 8/4，中等 4/2，移动平台 2/1

Position Iterations vs Velocity Iterations（XPBD）：
  Position Iterations:        8  ← 位置约束迭代（精度）
  Velocity Iterations:        1  ← 速度修正迭代（能量）

Chaos Broadphase Settings：
  AABB Tree Leaf Capacity:    16（每个叶节点最多 N 个物体）
  → 过小：树太深，遍历慢
  → 过大：叶节点碰撞对太多，窄相检测慢
  建议：8~32

Chaos Collision Margin：
  默认 0.01（1% 碰撞形状大小）
  微小间隙，防止接触抖动
  过大：物体悬浮在表面上方（不贴合）
  过小：接触抖动
```

---

## 四、多物理场景（Advanced）

```
UE 支持同一 World 中有多个 Physics Scene（高级用法）：

默认：每个 World 一个 FPhysScene_Chaos

多物理场景用例：
  1. 玩家角色 vs 布景分离
     主场景：角色/物体物理（高精度）
     布景场景：碎片/布料（低精度）
  
  2. 过场动画 vs 游戏物理分离
     不同求解参数

  3. 物理资产预览
     编辑器中 PhAT 使用独立物理场景

注意：多场景之间物体不能直接碰撞！
  → 需要手动同步位置（代价高）
  → 大多数项目不需要多物理场景
```

---

## 五、延伸阅读

- 📄 [1.10 物理调试可视化](./10-debug-visualization.md)
- 🔗 [Physics Settings](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-settings-in-unreal-engine)
