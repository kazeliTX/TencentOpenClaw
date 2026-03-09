# 6.9 Motion Matching vs 状态机：对比与混合

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、对比总览

| 维度 | 状态机（State Machine）| Motion Matching |
|------|----------------------|-----------------|
| 设计复杂度 | 随状态数量指数增长 | 数据库 + 权重配置 |
| 动画数量 | 少（每状态 1 个）| 多（需要各种变体）|
| 过渡自然度 | 依赖手工过渡动画 | 自动找最接近帧，自然 |
| 响应输入 | 快（直接切换）| 取决于搜索精度 |
| 调试难度 | 直观（状态图）| 较抽象（特征向量）|
| Mocap 需求 | 低 | 高（推荐 Mocap）|
| UE 成熟度 | 非常成熟 | UE5.4+ 较新 |
| 适用项目 | 中小型/手K动画 | 大型/写实/Mocap |

---

## 二、混合使用策略（推荐）

```
实际项目中，MM 和状态机经常混合使用：

架构：

  [Motion Matching]           ← 日常移动（Locomotion）
        │                        Walk, Run, Idle, Start, Stop
        │
  [Layered Blend]
    Base: ← MM 输出
    Blend: ← [State Machine]  ← 特殊动作（Action Layer）
                                  攻击、翻滚、跳跃、攀爬
                                  （仍用 Montage + 状态机控制）
        │
  [Output Pose]

分工：
  Motion Matching：负责 Locomotion（多方向移动，响应自然）
  状态机 + Montage：负责 Action（攻击/特技，精确控制）
```

---

## 三、何时选择 MM，何时用状态机

```
选 Motion Matching：
  ✅ 项目有高质量 Mocap 数据
  ✅ 角色需要多方向、多速度自然过渡
  ✅ 团队愿意投入配置/调试时间
  ✅ 写实 AAA 游戏

选 状态机：
  ✅ 手K动画（精细控制每个过渡）
  ✅ 风格化游戏（夸张动作）
  ✅ 动画数量有限（<50个）
  ✅ 原型/独立游戏（快速迭代）
  ✅ 所有"特殊动作"（攻击/技能）永远用状态机

MM + 状态机 混合：
  ✅ 大多数 AA/AAA 游戏的推荐方案
```

---

## 四、从状态机迁移到 MM 的步骤

```
1. 保留现有的 Action 状态机（攻击/翻滚不动）
2. 将 Locomotion 部分（Walk/Run/Idle）替换为 MM 节点
3. 创建 Pose Search Database，添加原有的 Walk/Run 动画
4. 添加 Character Trajectory Component 到角色
5. 配置 Pose Search Schema（位置+轨迹特征）
6. 在 AnimGraph 中：原 Locomotion State Machine → MM 节点
7. 调试权重，直到效果与原来相当或更好
8. 逐步增加动画数量，提升 MM 质量
```

---

## 五、延伸阅读

- 🔗 [Motion Matching 官方](https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-matching-in-unreal-engine)
- 🔗 [Motion Matching 知乎深度](https://zhuanlan.zhihu.com/p/1927668362969880273)
