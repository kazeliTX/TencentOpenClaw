# 1.4 物理 Actor 配置详解

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 35 分钟

---

## 一、启用物理模拟的必要条件

```
要让一个 Component 参与物理模拟，需要同时满足：

条件 1：有碰撞形状
  Collision → Collision Presets ≠ NoCollision
  至少有一个 Simple Collision（盒/球/胶囊/凸包）

条件 2：启用 Simulate Physics
  Details → Physics → Simulate Physics = true
  
条件 3：质量 > 0
  Mass > 0（默认会根据体积和密度自动计算）

条件 4：不是 Static 的 Mobility
  Mobility = Movable（Static/Stationary 无法参与动态物理）

检查方式：
  运行时：p.Chaos.DebugDraw.ShowCollision 1
  → 蓝色碰撞体 = 参与物理模拟
  → 灰色碰撞体 = 静态碰撞（不模拟，只做阻挡）
```

---

## 二、质量（Mass）配置

```
自动质量计算：
  Details → Physics → Mass (kg) → Override Mass = false（默认）
  Chaos 根据：
    体积（碰撞形状体积）
    密度（Physics Material 中设置，默认 1.0 g/cm³）
    自动计算质量
  
  质量 = 密度 × 体积

手动覆盖质量：
  Override Mass = true → 手动输入质量值（kg）
  
  建议值参考：
    小石头：0.5 ~ 2 kg
    木箱：  10 ~ 50 kg
    汽车：  1000 ~ 2000 kg
    角色：  70 ~ 100 kg

质量对物理的影响：
  质量越大：
    被力推动越难（F=ma，相同F，大质量加速小）
    碰撞时对轻物体影响越大（动量守恒）
    重力效果相同（自由落体与质量无关！）
    但抗冲量更强（J=m*Δv，相同J，大质量速度变化小）
```

---

## 三、阻尼（Damping）配置

```
Linear Damping（线性阻尼）：
  减缓平移速度，模拟空气阻力
  范围：0.0（无阻尼）~ ∞
  
  v(t) = v₀ * e^(-d*t)  （指数衰减）
  d = Linear Damping
  
  建议值：
    太空（无阻力）：0.0
    空气中：0.01 ~ 0.1
    水中（轻）：1.0 ~ 5.0
    粘稠液体：10.0 ~ 50.0
    立即停止：100.0+

Angular Damping（角阻尼）：
  减缓旋转速度，模拟旋转摩擦
  
  建议值：
    陀螺（低摩擦）：0.0
    正常物体：0.05 ~ 0.5
    快速稳定旋转：2.0 ~ 5.0

实践技巧：
  项目 → 物理 → 默认阻尼：可设全局默认值
  单个 Component 的设置会覆盖全局默认

C++ 动态设置：
  Comp->SetLinearDamping(0.5f);
  Comp->SetAngularDamping(1.0f);
```

---

## 四、锁定轴（Lock Flags）

```
锁定特定轴的位移或旋转：

Details → Physics → Constraints：
  Lock Position X / Y / Z    ← 锁定沿轴平移
  Lock Rotation X / Y / Z    ← 锁定绕轴旋转

典型用例：

  2D 游戏（Y 轴对齐）：
    Lock Position Y = true
    Lock Rotation X = true
    Lock Rotation Z = true
    → 物体只在 XZ 平面内运动

  乒乓球（不允许脱出球桌）：
    Lock Position Y = true
    → 球在 XZ 平面弹跳

  转门/旋转杆（只绕 Z 旋转）：
    Lock Position X/Y/Z = true
    Lock Rotation X/Y = true
    → 只允许绕 Z 旋转

C++ 设置：
  FBodyInstance* BI = Comp->GetBodyInstance();
  BI->bLockXTranslation = true;
  BI->bLockZRotation = true;
  BI->CreateDOFLock();
```

---

## 五、CCD（连续碰撞检测）

```
问题（Tunneling/穿透）：
  高速物体在一帧内穿越薄壁
  
  帧N：  物体在墙左侧
  帧N+1：物体在墙右侧  ← 错误！中间穿墙了
  
  原因：离散碰撞检测（每帧检查位置快照）
        高速 + 薄壁 = 快照之间漏掉碰撞

CCD（Continuous Collision Detection）：
  在帧之间进行射线/扫掠测试
  确保不遗漏任何碰撞
  
  UE 中启用：
    Details → Physics → Enable CCD = true
  
  CCD 类型：
    Linear CCD：沿线性路径扫掠（平移）
    Speculative CCD（UE5 默认）：
      预测下一帧位置，提前约束
      开销更低，但对旋转碰撞不完整

  CCD 性能开销：约 2~5x 普通碰撞检测
  
  建议：
    ✅ 子弹/飞射物（高速，小尺寸）必须开 CCD
    ✅ 薄板/薄墙的穿透问题
    ❌ 大型慢速物体（不需要，开了浪费）
    ❌ 大量小物体（堆叠箱子等，性能代价太高）

替代方案（CCD 性能影响太大时）：
  1. 增大碰撞体尺寸（让物体"看起来"更厚）
  2. 降低物体最大速度限制
  3. 用 Substepping 提高物理精度
```

---

## 六、Sleep 与 Wake

```
Sleep（休眠）：
  当物体速度/角速度低于阈值时，自动进入 Sleep
  Sleep 中的物体不参与物理计算 → 大幅节省性能

Sleep 阈值（全局设置）：
  Project Settings → Physics:
    Sleep Linear Velocity Threshold: 1.0 cm/s（默认）
    Sleep Angular Velocity Threshold: 0.05 rad/s（默认）
    Sleep Energy Threshold: 0.05（默认）
  
  注意：这三个阈值同时满足才会 Sleep

强制 Sleep/Wake（C++）：
  Comp->PutRigidBodyToSleep();  // 强制 Sleep
  Comp->WakeRigidBody();        // 强制 Wake
  bool bAsleep = Comp->RigidBodyIsAsleep();

Sleep 事件（蓝图）：
  On Component Sleep  ← 物体进入 Sleep 时触发
  On Component Wake   ← 物体从 Sleep 唤醒时触发

常见 Bug：物体"浮空静止"
  原因：阈值设置不当，物体在微小运动中反复 Sleep/Wake
  修复：
    1. 增大 Sleep Threshold
    2. 增大 Linear/Angular Damping 让物体更快停下
    3. 检查是否有持续力（重力异常/碰撞抖动）
```

---

## 七、物理材质对 Actor 的影响

```
Physics Material 控制表面物理属性：

Friction（摩擦）：
  Static Friction:  0.7（默认）← 开始滑动的阻力
  Dynamic Friction: 0.7（默认）← 滑动中的阻力
  Friction Combine: Average（两个材质取平均）

Restitution（弹性/恢复系数）：
  0.0 = 不弹（橡皮泥落地）
  0.3 = 低弹（普通橡胶）
  0.7 = 高弹（弹力球）
  1.0 = 完全弹性（理想弹性，实际不存在）
  Restitution Combine: Average

Density（密度，用于自动质量计算）：
  默认：1.0 g/cm³（水的密度）
  钢铁：7.8 g/cm³
  木材：0.5 g/cm³
  橡胶：1.1 g/cm³

赋值方式：
  Static Mesh → Details → Physics → Phys Material Override
  或在 Material 的 Physical Material 槽中设置
```

---

## 八、延伸阅读

- 📄 [1.5 质量与惯性张量](./05-mass-inertia.md)
- 📄 [代码示例：运行时物理控制](./code/01_enable_physics.cpp)
- 🔗 [Physics Bodies 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-bodies-in-unreal-engine)
