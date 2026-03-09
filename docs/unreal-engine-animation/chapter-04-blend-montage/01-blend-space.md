# 4.1 Blend Space 完全指南

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、Blend Space 的本质

Blend Space（混合空间）将多个动画按照参数空间排列，根据运行时参数值**自动插值混合**：

```
2D Blend Space 示意（Speed × Direction）：

         -180°   -90°    0°    +90°  +180°
  600  [↙跑]  [←跑]  [↑跑]  [→跑]  [↙跑]
  300  [↙走]  [←走]  [↑走]  [→走]  [↙走]
    0  [待机]  [待机]  [待机]  [待机]  [待机]
       ←────────── Direction ──────────→

运行时 Speed=450, Direction=45°：
  UE 在最近的 4 个采样点之间做双线性插值
  输出：↗跑（混合了 ↑跑 33% + →跑 67%）
```

---

## 二、Blend Space 类型

| 类型 | 轴数 | 典型用途 |
|------|------|---------|
| Blend Space | 2D | 方向运动（Speed + Direction）|
| Blend Space 1D | 1D | 单参数混合（速度、血量等）|
| Aim Offset | 2D | 瞄准方向叠加（Pitch + Yaw）|
| Aim Offset 1D | 1D | 单轴瞄准 |

---

## 三、创建 Blend Space

```
内容浏览器 → 右键 → Animation → Blend Space
选择 Skeleton → 命名：BS_Locomotion

编辑器界面：
┌──────────────────────────────────────────────┐
│  参数轴设置（左上角）                           │
│  Horizontal Axis: Direction (-180 ~ 180)     │
│  Vertical Axis:   Speed (0 ~ 600)            │
├────────────────────────────────────────────  │
│  采样点网格（主视口）                           │
│  拖入动画序列 → 放置到对应坐标位置               │
│                                              │
│  ● 坐标 (0, 0)     → AS_Idle                │
│  ● 坐标 (0, 300)   → AS_Walk_Fwd            │
│  ● 坐标 (90, 300)  → AS_Walk_Right          │
│  ● 坐标 (-90, 300) → AS_Walk_Left           │
│  ● 坐标 (0, 600)   → AS_Run_Fwd             │
│  ● ...                                      │
└──────────────────────────────────────────────┘
```

---

## 四、关键参数配置

### 4.1 轴设置

```
Axis Settings（每个轴的配置）：
  Name:             Speed
  Minimum Axis Value: 0.0
  Maximum Axis Value: 600.0
  Number of Grid Divisions: 4（插值网格精度）
  Snap to Grid:     ✅（采样点对齐到网格）
  Wrapping:         Clamp（不循环）或 Wrap（-180/+180 循环）
  
  Direction 轴特别注意：
    Wrap: ✅（允许 -180 和 +180 连续过渡）
    这让角色向左背跑和向右背跑之间有连续过渡
```

### 4.2 预览与测试

```
在编辑器视口中：
  底部拖动滑块 → 实时预览混合结果
  检查关键帧之间是否有穿模、滑步等问题

常见问题：
  • 转向时脚步滑：检查各方向动画的循环对齐（步频一致）
  • 速度变化时跳变：采样点间距太大，添加中间采样点
  • 上坡/下坡混合：添加第三轴（Slope Angle）→ 需要 3D BlendSpace（不常用）
```

---

## 五、采样点布局建议

```
8 方向移动 Blend Space 采样点布局（推荐）：

Speed=0（静止）：
  1 个 Idle 采样点，放在 (0, 0)

Speed=300（步行）：
  5 个采样点：
  (0, 300)     Walk_Fwd
  (90, 300)    Walk_Right
  (-90, 300)   Walk_Left
  (180, 300)   Walk_Bwd（或 Walk_Back）
  (-180, 300)  Walk_Back（Wrap 时与 180 相同）

Speed=600（跑步）：
  5 个采样点：
  (0, 600)     Run_Fwd
  (90, 600)    Run_Right
  (-90, 600)   Run_Left
  (180, 600)   Run_Bwd
  (-180, 600)  Run_Back

总计：11 个采样点
优化：手机端可简化为 5 个（仅正前方 + 四个对角）
```

---

## 六、在 AnimGraph 中使用

```
节点：Blend Space Player
  Blend Space: BS_Locomotion（拖入你的 BS 资产）
  
  输入引脚：
    Horizontal: AnimInstance.Direction   （浮点变量）
    Vertical:   AnimInstance.GroundSpeed （浮点变量）
  
  高级设置：
    Loop Animation: ✅（循环播放）
    Interpolation Type: Averaged（插值类型，Averaged 最稳定）
    Target Weight Interpolation Speed Per Sec: 5.0
      （采样点权重变化速度，越高 = 响应越快但可能抖动）
```

---

## 七、同步步频（Step Sync）

```
重要：所有同一 Blend Space 中的动画应有相同的步频！

错误示例：
  Walk_Fwd：步频 1.0Hz（1秒迈一步）
  Run_Fwd：  步频 2.5Hz（1秒迈两步半）
  在 Walk→Run 过渡时，脚步明显不同步

解决方案：
1. 让动画师确保步频一致（最佳方案）
2. 使用 Sync Groups（见 4.8 章节）
3. 对跑步动画调整播放速率（PlayRate）以匹配步频

配置 Sync Groups in Blend Space：
  BlendSpace 设置 → Sync Group Name: SyncGroupLoco
  所有使用同一 Sync Group 的动画会同步相位
```

---

## 八、延伸阅读

- 📄 [4.8 Sync Groups 动画同步](./08-sync-groups.md)
- 🔗 [Blend Space 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/blend-spaces-in-unreal-engine)
- 🔗 [Locomotion Blend Space 教程（知乎）](https://zhuanlan.zhihu.com/p/381967985)
