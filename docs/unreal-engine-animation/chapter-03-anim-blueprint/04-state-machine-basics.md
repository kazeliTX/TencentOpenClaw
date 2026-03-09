# 3.4 状态机基础与设计模式

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 35 分钟

---

## 一、什么是 Animation State Machine

Animation State Machine（动画状态机）是 AnimGraph 中的核心逻辑节点，用于根据角色状态选择不同的动画姿势：

```
状态机本质：有限状态自动机（FSM）

每个状态（State）：
  • 对应一个动画姿势（如：Idle、Walk、Run、Jump）
  • 包含一个或多个动画节点（Sequence Player、BlendSpace等）
  
每个过渡（Transition）：
  • 定义从一个状态切换到另一个状态的条件
  • 包含过渡时长和混合曲线

任何时刻只有一个（或极少数）激活状态
激活状态的权重逐渐从 0 → 1（过渡期间同时有两个激活状态）
```

---

## 二、经典角色状态机设计

### 2.1 单层状态机（简单角色）

```
[Idle] ←→ [Walk] ←→ [Run]
  ↕              ↕
[Jump]       [Jump]
  ↕
[Fall]
  ↓
[Land]

适用：简单角色，无战斗系统
缺点：状态增多后连线混乱，难以维护
```

### 2.2 分层状态机（推荐）

```
结构：
  Main State Machine（主状态机）
    ├── Ground（地面层）
    │     ├── Locomotion SM（运动子状态机）
    │     │     ├── Idle
    │     │     ├── Walk
    │     │     ├── Run
    │     │     └── Sprint
    │     └── Crouch SM（蹲伏子状态机）
    │           ├── CrouchIdle
    │           └── CrouchWalk
    └── Air（空中层）
          ├── Jump
          ├── Fall
          └── Land

优点：
  • 层级清晰，每层只处理自己的逻辑
  • 子状态机可以独立设计和调试
  • 添加新状态不影响其他层级
```

### 2.3 UE5 推荐架构（Linked AnimGraph）

```
Main AnimBP
  └── Locomotion Linked AnimBP    (处理移动动画)
  └── Combat Linked AnimBP       (处理战斗动画)
  └── IK Linked AnimBP           (处理IK)
  └── Additive Linked AnimBP     (处理叠加动画)

State Machine 只负责选择"应该播放什么动画"
Linked AnimBP 负责具体的混合细节
```

---

## 三、状态机节点详解

### 3.1 创建状态机

```
在 AnimGraph 中：
  右键 → Add State Machine
  命名：SM_Locomotion（下划线 + 功能名的命名习惯）

双击进入状态机编辑器：
  初始状态：Entry（自动创建，不可删除）
  第一个状态：通常从 Entry → Idle 的自动过渡开始
```

### 3.2 添加状态（State）

```
在状态机中：
  右键 → Add State
  命名：Idle（状态名使用 PascalCase）

双击进入状态内部：
  拖入 Sequence Player 节点
  设置 Sequence：AS_Idle_Breathing（动画序列资产）
  连接到 Output Animation Pose
```

### 3.3 条件状态（Conduit）

```
Conduit（管道状态）：不播放动画，仅用于分支判断

示例：Fall 状态根据下落速度分支到不同落地动画

  [Fall] → [Land Conduit] → [LandSoft]（轻落地）
                         → [LandHard]（重落地）

Land Conduit 内部：
  条件 → LandSoft：FallSpeed < 600
  条件 → LandHard：FallSpeed >= 600

优点：避免一个大状态内部做复杂分支，逻辑更清晰
```

---

## 四、过渡（Transition）设置

### 4.1 基本过渡

```
添加过渡：
  从状态 A 拖出到状态 B → 自动创建过渡箭头

过渡设置（双击过渡箭头）：
  Duration: 0.2s（过渡时长，混合两个姿势的时间）
  Blend Logic: Custom Blend Curve（自定义曲线）或 Standard Blend
  
过渡规则（双击过渡上的圆形图标进入）：
  添加你的条件节点
  例：Speed > 10 → 从 Idle 过渡到 Walk
```

### 4.2 过渡优先级

```
当一个状态有多个出去的过渡时：
  优先级 = 过渡在列表中的顺序（从上到下）

重要：确保互斥条件的优先级正确
  ❌ 错误顺序：
    [1] Speed > 0   → Walk（永远会先触发，跑步永远不会）
    [2] Speed > 300 → Run
    
  ✅ 正确顺序：
    [1] Speed > 300 → Run
    [2] Speed > 0   → Walk
```

---

## 五、角色完整状态机示例

```
典型第三人称角色状态机：

Entry
  └─→ Ground
         ├── Idle ←→ Locomotion（Ground BlendSpace）
         │     ↑↓ Speed
         │
         ├── Crouch_Idle ←→ Crouch_Walk
         │     ↑↓ bIsCrouching
         │
         ├──→ Jump_Start（单次播放）
         │         ↓ 动画结束
         │    Jump_Loop（循环）
         │         ↓ bIsInAir = false
         │    Jump_Land（单次播放）
         │         ↓ 动画结束
         │   回到 Idle/Locomotion
         │
         └──→ Death（单次播放，不可返回）

过渡条件汇总：
  Ground → Jump:   bIsInAir = true
  Jump_Start → Jump_Loop: 动画接近结束（Time Remaining < 0.1）
  Jump_Loop → Jump_Land:  bIsInAir = false
  Jump_Land → Idle: 动画完成
  Any → Death:     bIsDead = true（全局过渡，不受其他规则影响）
```

---

## 六、"全局"过渡（Any State）

```
Any State 节点：让某个过渡从任何状态都能触发

示例：死亡动画从任何状态都能触发

设置：
  State Machine 编辑器 → 右键 → Add Any State
  从 Any State → Death 添加过渡
  过渡条件：bIsDead == true

注意：
  Any State 过渡的 Duration 通常设为 0（立即切换）
  或极短（0.1s），避免死亡动画混合不自然
```

---

## 七、延伸阅读

- 📄 [3.5 状态机进阶：条件、别名、子状态机](./05-state-machine-advanced.md)
- 📄 [3.6 过渡规则与混合曲线](./06-transition-rules.md)
- 🔗 [State Machine 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/state-machines-in-unreal-engine)
