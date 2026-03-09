# 6.1 Root Motion 基础原理

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、为什么需要 Root Motion

```
游戏角色移动有两种驱动方式：

方式 A：CharacterMovement 驱动（代码控制）
  位置 = 上一帧位置 + Velocity × DeltaTime
  动画只是"表演"，不决定实际位移
  
  优点：精确控制速度/加速度，网络同步容易
  缺点：动画滑步（动画脚步节奏与实际速度不匹配）

方式 B：Root Motion 驱动（动画数据控制）
  位置 = 动画根骨骼的位移数据
  动画的位移数据直接驱动角色在世界中的运动
  
  优点：动画和位移完美同步（攻击跳跃翻滚无滑步）
  缺点：网络同步复杂，需要 CharacterMovement 配合
```

---

## 二、Root Motion 的数据来源

```
骨骼层级：
  root（根骨骼）
    └─ pelvis（骨盆）
         ├─ spine_01
         │    └─ ...（上半身）
         ├─ thigh_l
         │    └─ ...（左腿）
         └─ thigh_r
              └─ ...（右腿）

Root Motion 的数据：
  动画中 root 骨骼的帧间位移和旋转量
  
  例：翻滚动画
    帧 0: root 在 (0, 0, 0)
    帧 5: root 在 (50, 0, 10)
    帧 10: root 在 (120, 0, 0)
    → 每帧的 root 位移 = 这一帧 CharacterMovement 的输入
```

---

## 三、Root Motion 处理管线

```
Root Motion 数据流（每帧）：

1. AnimInstance 更新动画（Worker Thread）
   → 计算根骨骼的帧间位移 ΔPosition

2. 主线程：从 AnimInstance 提取 Root Motion
   UCharacterMovementComponent::ConsumeRootMotion()
   → 得到 FRootMotionMovementParams

3. CharacterMovement::PerformMovement()
   → 将 Root Motion 位移加入到当前速度/位置计算中
   → 执行碰撞检测（仍然遵循物理碰撞）
   
4. 输出：角色实际移动（既有动画位移，又有碰撞响应）
```

---

## 四、Root Motion Lock 模式

```
Root Motion Root Lock 决定动画开始/结束时根骨骼的基准位置：

Ref Pose（推荐）：
  根骨骼锁定在参考姿势位置
  适合大多数动作动画（翻滚、攻击）
  
Anim First Frame：
  根骨骼锁定在动画第一帧位置
  
Zero（旧版默认）：
  根骨骼锁定在原点

设置位置：
  AnimSequence → Details → Root Motion Root Lock
```

---

## 五、Root Motion 的三种模式

```
UE 中 Root Motion 有三种激活模式（AnimInstance → Details）：

1. No Root Motion Extraction（默认）
   不提取 Root Motion，纯动画表演
   
2. Ignore Root Motion（提取但忽略）
   提取 Root Motion 数据但不应用到角色
   → 适合测试动画位移量

3. Root Motion from Everything（推荐）
   所有动画（包括 Blended）都产生 Root Motion
   CharacterMovement 应用所有 Root Motion

4. Root Motion from Montages Only
   只有 Montage 动画产生 Root Motion
   普通循环动画（行走）仍由 CharacterMovement 驱动
   → 适合"日常移动=代码控制，特殊动作=Root Motion"的混合方案
```

---

## 六、延伸阅读

- 📄 [6.2 Root Motion 配置与工作流](./02-root-motion-config.md)
- 📄 [6.3 Root Motion 网络同步](./03-root-motion-network.md)
- 🔗 [Root Motion 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/root-motion-in-unreal-engine)
