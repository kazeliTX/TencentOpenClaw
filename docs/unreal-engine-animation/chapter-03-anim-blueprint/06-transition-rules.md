# 3.6 过渡规则与混合曲线

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、过渡的本质

```
过渡（Transition）不是"立即切换"，而是在 Duration 时间内
同时激活两个状态并线性（或曲线）插值其权重：

时间轴：
  t=0.0s: StateA=1.0, StateB=0.0   （过渡开始）
  t=0.1s: StateA=0.5, StateB=0.5   （混合中间）
  t=0.2s: StateA=0.0, StateB=1.0   （过渡完成）
  
  过渡完成前，AnimGraph 同时评估两个状态并混合输出
  → 有性能开销，Duration 不宜过长（推荐 0.1~0.3s）
```

---

## 二、过渡参数详解

```
双击过渡箭头 → 过渡属性：

Duration: 0.2
  过渡时长（秒）
  推荐值：
    普通移动状态切换：0.2~0.3s
    攻击动画：0.1~0.15s（需要快速响应）
    跌倒/死亡：0.0s（立即切换）

Blend Logic: Standard Blend
  Standard Blend：均匀线性混合（默认，大多数情况够用）
  Custom Blend Curve：使用自定义曲线（见下方）
  Inertialization：惯性化混合（UE5 推荐，见下方）

Sync Group Name: SyncGroupLoco
  同步组（见第四章 Montage 部分详解）
  
Blend Profile:
  按骨骼分配不同过渡权重
  例：上半身过渡快（0.05s），下半身过渡慢（0.2s）
```

---

## 三、自定义混合曲线

```
Custom Blend Curve 允许用曲线精确控制混合进度：

标准线性曲线：
  权重
  1.0│           ╱────
     │        ╱
  0.5│    ╱
     │ ╱
  0.0│────
     └──────────── 时间
      0.0   0.5   1.0

Ease In 曲线（先慢后快）：
  权重
  1.0│        ╭────
     │      ╭╯
  0.5│    ╭╯
     │  ──╯
  0.0│────
  效果：过渡开始时看起来更自然

Ease Out 曲线（先快后慢）：
  适用：落地动画（快速贴地，然后缓慢稳定）

配置方法：
  Custom Blend Curve → 点击曲线 → 打开曲线编辑器
  添加关键帧，调整切线
```

---

## 四、Inertialization（惯性化混合）

UE5 引入的新过渡方式，视觉质量远优于传统混合：

```
传统混合问题：
  • 硬混合：两个姿势直接权重叠加，可能有滑步、穿帮
  • 过长过渡：动画感觉"飘"
  
Inertialization 原理：
  不混合两个姿势，而是：
  1. 记录当前姿势的速度（velocity）
  2. 切换到新状态（新动画立即全权重）
  3. 用弹簧（Spring）函数平滑消除跳变

效果对比：
  传统混合：两个动画的骨骼位置线性插值（可能造成奇怪中间姿势）
  Inertialization：新动画立即开始，初始速度继承旧动画（更自然）

配置方法：
  1. AnimGraph 中添加 "Inertialization" 节点（放在 Output Pose 前）
  2. 过渡属性 → Blend Logic: Inertialization
  3. Duration: 0.2（现在控制惯性消散时间，而非混合时间）

注意：
  • 所有连接到 Inertialization 节点之前的过渡都要设为 Inertialization
  • 传统混合和 Inertialization 不能混用（同一 SM 内）
```

---

## 五、Blend Profile（分骨骼混合速度）

```
Blend Profile 允许不同骨骼以不同速度过渡：

配置位置：
  Skeleton 编辑器 → Blend Profiles → New Profile

示例：武器切换动画
  上半身（持枪动画）：过渡快（手臂需要快速到位）
  下半身（站立/移动）：过渡慢（腿部不需要那么快响应）

设置值（0.0~1.0，相对于 Duration 的缩放）：
  root: 1.0
  spine_01: 1.0
  spine_03: 0.5（上半身减半：快 2 倍）
  upperarm_l: 0.3（手臂更快）
  thigh_l: 1.0（腿部保持原速）

在过渡属性中：
  Blend Profile: MyBlendProfile_WeaponSwitch
```

---

## 六、调试过渡

```
如何检查过渡是否按预期工作：

1. AnimBP 编辑器 PIE：
   选择调试实例 → 状态机编辑器中高亮显示激活状态
   过渡箭头会显示当前规则是否满足（绿色 = 满足条件）

2. 控制台：
   ShowDebug Animation
   → 显示 State、Transition、Weight

3. 添加调试打印（临时）：
   在过渡规则末尾添加 Print String 节点
   （过渡条件满足时打印）
   注意：打印节点在过渡完成后移除

4. 慢动作调试：
   Global Time Dilation 设为 0.2
   → 方便观察过渡混合过程
   控制台：slomo 0.2
```

---

## 七、延伸阅读

- 🔗 [State Machine Transitions 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/transition-rules-in-unreal-engine)
- 🔗 [Inertialization 详解](https://dev.epicgames.com/documentation/en-us/unreal-engine/inertialization-in-unreal-engine)
- 🔗 [Blend Profiles 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/blend-profiles-in-unreal-engine)
