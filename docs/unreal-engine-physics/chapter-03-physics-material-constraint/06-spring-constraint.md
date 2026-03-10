# 3.6 弹簧约束（Spring Constraint）

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 40 分钟

---

## 一、弹簧约束的物理模型

```
理想弹簧（胡克定律）：
  F = -k * x
  k = 弹簧刚度（Spring Stiffness）
  x = 偏离平衡位置的距离（位移）
  F = 恢复力（指向平衡位置）

带阻尼的弹簧（更真实）：
  F = -k * x - c * v
  c = 阻尼系数（Damping）
  v = 当前速度
  
  临界阻尼条件：c = 2 * sqrt(k * m)
    欠阻尼（c < 临界）：振荡，逐渐收敛
    临界阻尼（c = 临界）：最快无振荡收敛（理想门弹簧）
    过阻尼（c > 临界）：缓慢回归，不振荡（粘稠感）

UE 中的弹簧（Drive 模式）：
  Position Strength → 对应 k（弹簧刚度）
  Velocity Strength → 对应 c（阻尼）
  Maximum Force    → 最大弹簧力（防止无限大）
  
  临界阻尼计算（需要 C++）：
    float CriticalDamping = 2.f * FMath::Sqrt(Stiffness * Mass);
    Constraint->SetLinearDriveParams(Stiffness, CriticalDamping, MaxForce);
```

---

## 二、位置弹簧（Linear Spring）

```
线性弹簧：沿某轴弹回到目标位置

配置：
  Linear X Motion：Free（X 轴自由，由弹簧控制，不是 Locked）
  Linear Y/Z Motion：Locked（其他轴锁定）
  
  Linear Position Drive X：true
  Linear Position Target：FVector(0, 0, 0)（平衡位置，相对约束空间）
  Drive Position Strength X：500.f（弹簧刚度）
  Drive Velocity Strength X：50.f（阻尼）
  Linear Maximum Force：10000.f

应用场景：
  弹弓（Slingshot）：
    弹力小球被拉离平衡 → 弹簧约束恢复力发射小球
    
  悬挂弹簧（但 Chaos Vehicle 有专用系统更好）：
    轮子在凹凸路面上振动，弹簧约束回弹

  弹床/蹦床模拟（配合 Restitution=1 的PhysMat）：
    接触面是弹簧约束 → 接触时产生向上弹力
```

---

## 三、角度弹簧（Angular Spring）

```
旋转弹簧：偏离目标旋转时弹回

配置：
  Angular Twist Motion：Free 或 Limited
  Angular Drive Mode：Orientation Drive（朝向驱动）
  Target Orientation：FQuat::Identity（直立平衡位置）
  Drive Position Strength：200.f
  Drive Velocity Strength：30.f

应用场景：
  不倒翁（Weeble）：
    底部固定，顶部可摆动
    Angular Spring 驱使其回到直立状态
    
  布偶（Ragdoll 部分约束）：
    骨骼在 Ragdoll 时，弹簧约束驱使骨骼回到绑定姿势
    → "弹性布娃娃"效果（有弹性但仍能被击倒）
    
  车辆转向回正：
    方向盘/前轮的角度弹簧 → 松手后自动回正

弹簧强度调优示例（不倒翁）：
  // 太弱：被轻触就倒且回不来
  // 太强：几乎不能推倒，感觉硬塑料
  // 合适：被推倒后优雅回弹
  Constraint->SetAngularDriveParams(
      200.f,   // Position Strength（倾向直立的弹力）
      20.f,    // Velocity Strength（防止振荡的阻尼）
      5000.f   // Max Force
  );
```

---

## 四、弹簧链（多段弹簧）

```
多个刚体用弹簧串联 → 绳子/弹性链条效果

架构：
  Body[0] ←弹簧→ Body[1] ←弹簧→ Body[2] ←弹簧→ ... Body[N]
  
  每对相邻 Body 之间：
    Linear Motion：Free + Linear Position Drive（弹簧）
    Angular Motion：Free + Angular Drive（角度弹簧，控制弯曲刚度）

关键参数：
  Linear Stiffness（线性刚度）：控制拉伸/压缩的弹力
    高值 → 绳子不容易拉伸（更像刚性链条）
    低值 → 绳子容易拉伸（弹性绳）
  
  Angular Stiffness（角度刚度）：控制弯曲的弹力
    高值 → 绳子较硬（不容易弯曲）
    低值 → 绳子柔软（容易下垂/弯曲）
  
  Damping：防止弹簧振荡
    太小 → 绳子一直抖动
    太大 → 绳子像果冻，运动迟缓

性能注意：
  N段弹簧链 → N个约束，每帧求解
  弹性高 + 阻尼低 → 需要更多子步进才稳定
  实际游戏绳索推荐使用：UCableComponent（引擎内置，比物理绳索高效）
```

---

## 五、弹簧约束 Bug 与修复

```
Bug 1：弹簧振荡不收敛（爆炸）
  原因：Position Strength 远大于 Velocity Strength * 2 / sqrt(mass)
  修复：增大 Velocity Strength 到临界阻尼值
        或降低 Position Strength

Bug 2：弹簧到达目标后仍然抖动
  原因：Sleep Threshold 太高或未休眠
  修复：在 Drive 结束后手动设置低阻尼（让它平静下来）

Bug 3：弹簧力不够（达不到目标位置）
  原因：Maximum Force 太小，质量太大
  修复：增大 Maximum Force；或降低质量；或增大 Position Strength

Bug 4：多段弹簧链数值爆炸（高速旋转）
  原因：角约束累积误差，步长太大
  修复：开启 Substepping（每帧 4~6 子步）
```

---

## 六、延伸阅读

- 📄 [3.7 滑动约束](./07-prismatic-constraint.md)
- 📄 [3.8 约束限制详解](./08-joint-limits.md)
- 🔗 [Physics Constraint Component Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-constraint-component-user-guide-in-unreal-engine)
