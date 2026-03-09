# 3.5 状态机进阶：条件、别名、子状态机

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、复杂过渡条件

### 1.1 时间剩余条件（Time Remaining）

```
用途：当动画快播放完时自动过渡（如：翻滚结束后回到移动）

节点：Time Remaining (ratio)
  输入：AnimSequence 节点引用
  输出：float（0.0 = 刚开始，1.0 = 播放完毕）
        也有 "Time Remaining" 输出秒数版本

典型配置（翻滚结束过渡）：
  Time Remaining (ratio) → Dodge_Animation → 连接到 ≤ 节点
  ≤ 节点：0.1（剩余 10% 时触发）
  → 过渡到 Locomotion

注意：需要将 State 内部的 Sequence Player 节点"提升"为引用，
      才能在过渡规则中访问它：
      Sequence Player → 右键 → Promote to Graph Output
      → 在过渡规则中用 "Get Relevant Anim Time Remaining" 节点
```

### 1.2 混合权重条件（Blend Weight）

```
用途：等待过渡完全完成后再做某些操作

节点：Get State Weight
  参数：State Machine Name, State Name
  输出：float（当前状态的激活权重，1.0 = 完全激活）

示例：等待 Idle 状态权重达到 0.95 时才允许播放特定动画
  Get State Weight("SM_Locomotion", "Idle") → ≥ 0.95
```

### 1.3 自动过渡（Automatic Transition）

```
用途：在特定状态停留一段时间后自动过渡

配置：
  过渡规则内：
    True（硬编码为 true）→ 过渡条件始终满足

  同时设置：
    过渡 → Cross-fade Duration: 0.5（让混合看起来自然）
    Should Fire in Reverse: ✅（双向过渡时用）

实际用途：
  • Jump_Start → Jump_Loop（动画播完自动切换）
  • Hit_Reaction → Locomotion（受击动画结束后自动恢复）
```

---

## 二、状态别名（State Alias）

UE5 引入状态别名，解决了"多个状态都要过渡到同一目标"的问题：

```
问题：没有别名时
  Idle → Jump
  Walk → Jump
  Run → Jump
  Sprint → Jump
  (需要 4 条过渡线，且过渡条件完全相同)

解决：使用 State Alias

1. 右键 → Add State Alias
2. 命名："Grounded"
3. 在 Alias 属性中勾选它代表的状态：
   ✅ Idle, ✅ Walk, ✅ Run, ✅ Sprint
4. 只需一条过渡：[Grounded Alias] → [Jump]
   过渡条件：bIsInAir == true

效果：上述 4 个状态中的任意一个，都会参与这条过渡规则
```

---

## 三、子状态机（Nested State Machine）

```
子状态机本质上是一个状态，但其内部又是一个完整的状态机

用途：
  1. 逻辑分组（将相关状态封装在一起）
  2. 复用（相同的子状态机可以在多个地方使用）
  3. 减少顶层状态机的复杂度

示例：Combat 子状态机

主状态机：
  Locomotion ←→ [Combat SM] ←→ Death

Combat SM 内部：
  Entry → Combat_Idle
  Combat_Idle ←→ Attack_1 → Attack_2 → Attack_3（连段）
  Combat_Idle ← Hit_Reaction（任意时刻可受击）
  Combat_Idle → Block

注意：子状态机有独立的 Entry 和 Exit，
      进入时从 Entry 开始，退出时从当前任意状态直接切换到外层过渡
```

---

## 四、动画层（Animation Layers）

UE5 引入动画层系统（Animation Layers），可以将 AnimGraph 的部分功能暴露为接口：

```
定义层（在 AnimBP Class Settings 中）：
  + Add Layer Interface → IAnimInterface_Combat
    Interface Functions:
      • FullBody()    → 全身动画层
      • UpperBody()   → 上半身动画层

在 AnimGraph 中：
  Layered Blend Per Bone
    Base Pose:    ← SM_Locomotion（下半身）
    Blend Pose 0: ← UpperBody()（接口层，可被替换）

在角色不同武器的子 AnimBP 中：
  实现 UpperBody() 层 → 提供对应武器的动画逻辑

切换武器时：
  Mesh->LinkAnimClassLayers(UAnimBP_Rifle::StaticClass());
  Mesh->LinkAnimClassLayers(UAnimBP_Sword::StaticClass());
```

---

## 五、实用状态机调试命令

```bash
# 显示所有角色的当前状态机状态
ShowDebug Animation

# 显示特定 Actor 的动画信息（在关卡中选中 Actor）
ShowDebug AnimationVerbose

# 打印状态机状态转换日志
# 在 AnimBP 节点上右键 → Enable Logging

# 查看混合树权重
# AnimBP 编辑器 PIE 调试 → Live Values Panel
```

---

## 六、延伸阅读

- 📄 [3.6 过渡规则与混合曲线](./06-transition-rules.md)
- 📄 [3.8 Linked AnimGraph 模块化](./08-linked-anim-graph.md)
- 🔗 [Animation Layers 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-linking-in-unreal-engine)
- 🔗 [State Alias 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/state-machine-node-reference-for-animation-blueprints-in-unreal-engine#statealias)
