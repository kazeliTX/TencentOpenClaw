# 7.8 Rigid Body 动画节点

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、Rigid Body 节点简介

```
Rigid Body 节点在 AnimGraph 中直接运行 Chaos 物理模拟：
  使用角色的 Physics Asset 作为物理体
  在动画线程（Worker Thread）中运行物理（非主线程）
  不会干扰 CharacterMovement 的正常物理

与布娃娃的区别：
  布娃娃：整个 Mesh 切换为物理，CharacterMovement 禁用
  Rigid Body 节点：物理在 AnimGraph 中作为一个节点运行
                   CharacterMovement 继续正常工作
                   可以混合物理和动画输出

典型用途：
  角色受击时的局部刚体运动（无需完整布娃娃）
  死亡时从动画平滑过渡到完整物理
  程序化次级运动（硬质配件的物理惯性）
```

---

## 二、配置 Rigid Body 节点

```
AnimGraph 节点：Rigid Body

参数配置：
  Simulation Space: Component Space（推荐）
    或 World Space（更准确但开销更高）
  
  Override Physics Asset: None（使用 Mesh 默认的 Physics Asset）
  
  Enable World Geometry: ✅
    （物理体与世界碰撞，否则穿地）
  
  Override World Gravity: ☐（使用默认重力）
  
  External Forces:
    从 AnimBP 变量读取外力（如风、爆炸冲击）

Alpha: 0.0~1.0
  0.0 = 完全动画（Rigid Body 无效果）
  1.0 = 完全 Rigid Body 物理

节点连接：
  Input: [动画姿势]（作为物理初始姿势和参考）
  Output: [物理驱动的姿势]
```

---

## 三、死亡过渡（动画 → Rigid Body → 布娃娃）

```
高质量死亡过渡流程：

阶段 1：播放死亡动画（0~0.3s）
  继续播放当前动画
  Rigid Body Alpha：0.0 → 0.5（逐渐引入物理）

阶段 2：Rigid Body 混合（0.3~0.8s）
  Rigid Body Alpha：0.5 → 1.0
  角色姿势逐渐完全由物理驱动
  CharacterMovement 仍然活跃

阶段 3：切换为完整布娃娃（0.8s+）
  SetSimulatePhysics(true)
  Rigid Body 节点不再需要（已经是物理了）

AnimGraph 配置：
  [死亡 Montage]
          │
  [Blend（Rigid Body Alpha）]
    A: 死亡 Montage
    B: [Rigid Body 节点]
  Alpha: DeathPhysicsAlpha（从 0 平滑到 1）
```

---

## 四、性能注意

```
Rigid Body 节点在 Worker Thread 运行物理，
但开销仍比纯动画高：

单角色开销（20个物理体）：~0.2~0.5ms
100个角色（都激活 Rigid Body）：可能高达 20~50ms

建议：
  只在需要时激活（Alpha > 0 时才运行物理）
  UE 会在 Alpha = 0 时自动跳过 Rigid Body 计算
  远距离角色（LOD2+）禁用 Rigid Body：LOD Threshold = 1
```

---

## 五、延伸阅读

- 🔗 [Rigid Body 节点文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/rigid-body-in-unreal-engine)
