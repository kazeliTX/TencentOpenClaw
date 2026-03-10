# 1.3 刚体基础概念

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 40 分钟

---

## 一、刚体定义

```
刚体（Rigid Body）：
  形状在运动过程中不发生形变的理想物体
  任意两点之间的距离保持不变

6 自由度（Degrees of Freedom, DOF）：
  平移：X / Y / Z（3 个）
  旋转：绕 X / Y / Z 轴（3 个）
  共 6 DOF → 完整描述刚体在 3D 空间的状态

状态向量（刚体完整状态）：
  位置      x ∈ ℝ³         （质心世界坐标）
  旋转      q ∈ ℍ（单位四元数）
  线速度    v ∈ ℝ³
  角速度    ω ∈ ℝ³

  共 13 个标量（位置3 + 四元数4 + 线速3 + 角速3）
```

---

## 二、牛顿第二定律在刚体中的应用

```
平移运动（线性）：
  F = m * a
  F = m * (dv/dt)
  
  → UE 中：AddForce(FVector Force)
    在物理帧中积分：v(t+Δt) = v(t) + F/m * Δt
    x(t+Δt) = x(t) + v(t) * Δt

旋转运动（角度）：
  τ = I * α
  τ = I * (dω/dt)
  
  τ（Torque，力矩）= r × F（叉积）
  I（Inertia Tensor，惯性张量）= 3x3 矩阵
  α（Angular Acceleration，角加速度）
  
  → UE 中：AddTorqueInDegrees(FVector Torque)
    在物理帧中积分：ω(t+Δt) = ω(t) + I⁻¹ * τ * Δt
    q(t+Δt) = q(t) + 0.5 * [0,ω] * q(t) * Δt（四元数积分）

UE 坐标系规范：
  X 轴：前方（Forward）
  Y 轴：右方（Right）
  Z 轴：上方（Up）
  
  力/速度单位：cm/s, cm/s²（UE 默认厘米）
  质量单位：kg
  角速度单位：rad/s（内部），deg/s（接口）
```

---

## 三、冲量（Impulse）vs 力（Force）

```
力（Force）：
  持续施加，在每个物理帧中积分
  效果：改变加速度 → 逐渐改变速度
  用途：重力、推进力、风力等持续力
  
  UE API：AddForce(F)
  效果：v += F/m * Δt（每帧）

冲量（Impulse）：
  瞬时施加，直接改变速度（跳过力→加速度→速度的积分）
  效果：速度瞬间改变 Δv = J/m
  用途：爆炸冲击、碰撞反弹、跳跃
  
  UE API：AddImpulse(J)
  效果：v += J/m（一次性）

力矩（Torque）vs 角冲量（Angular Impulse）：
  AddTorqueInDegrees(T)：持续旋转力矩
  AddAngularImpulseInDegrees(A)：瞬间角冲量
  
实践选择：
  物理效果需要帧率无关 → 优先用 Impulse
  持续推进（火箭、风扇）→ 用 Force（在 SubstepTick 中更准确）
```

---

## 四、线动量与角动量守恒

```
线动量：p = m * v
  封闭系统中线动量守恒
  碰撞时：p_before = p_after
  
  UE 中碰撞计算：
    Restitution（弹性系数 e）：
      e = 0：完全非弹性（粘性碰撞，不反弹）
      e = 1：完全弹性（完美反弹，能量守恒）
    碰撞后速度：
      v1' = (m1*v1 + m2*v2 + m2*e*(v2-v1)) / (m1+m2)
      v2' = (m1*v1 + m2*v2 + m1*e*(v1-v2)) / (m1+m2)

角动量：L = I * ω
  封闭系统中角动量守恒
  影响因素：
    质量分布（惯性张量 I）
    旋转速度（ω）
  
  实例：
    旋转的物体被击中 → 角动量重新分配
    AddImpulseAtLocation(Force, Location) 同时产生线冲量和角冲量

UE 中查看动量：
  FBodyInstance* BI = Comp->GetBodyInstance();
  FVector LinearMomentum  = BI->GetUnrealWorldVelocity() * BI->GetBodyMass();
  FVector AngularMomentum = BI->GetUnrealWorldAngularVelocityInRadians()
                            * BI->GetBodyInertiaTensor().X; // 简化
```

---

## 五、欧拉角 vs 四元数

```
欧拉角（Euler Angles）：
  用 Roll / Pitch / Yaw（RPY）或 XYZ 旋转顺序表示
  直观易理解，但有万向锁问题
  
  万向锁（Gimbal Lock）：
    当 Pitch = ±90° 时，Roll 和 Yaw 轴重合
    失去一个自由度，无法正确插值
  
  UE 中：FRotator（Pitch, Yaw, Roll in degrees）

四元数（Quaternion）：
  q = (w, x, y, z)，|q| = 1
  无万向锁问题
  插值（Slerp）平滑
  
  物理引擎内部使用四元数！
  
  UE 中：FQuat
  换算：FQuat = FRotator.Quaternion()
  
  四元数表示旋转：
    绕轴 n̂ 旋转角度 θ：
    q = (cos(θ/2), n̂_x * sin(θ/2), n̂_y * sin(θ/2), n̂_z * sin(θ/2))
  
  四元数乘法（组合旋转）：
    先旋转 q1，再旋转 q2：q_total = q2 * q1（注意顺序！）
  
  在 UE 物理中的应用：
    骨骼旋转/刚体旋转均用四元数内部表示
    获取旋转：BI->GetUnrealWorldTransform().GetRotation() → FQuat
```

---

## 六、质心（Center of Mass）

```
质心（CoM）= 质量分布的几何中心

质心位置公式：
  x_cm = (Σ m_i * x_i) / Σ m_i

质心在物理中的重要性：
  力作用在质心 → 只产生平移，不产生旋转
  力作用在非质心点 → 产生平移 + 旋转（力矩 = r × F）
  
  重要！UE 物理中所有速度/冲量的施加点影响角速度

UE 中的质心设置：
  默认：Chaos 自动根据碰撞形状计算质心
  
  手动偏移（当默认质心不理想时）：
    Detail Panel → Physics → Center of Mass Offset
    或 C++：BodyInstance.COMNudge = FVector(0, 0, -10);
    → 将质心向下移动 10cm（如车辆降低重心）
  
  查看质心：
    p.Chaos.DebugDraw.ShowCenterOfMass 1

典型应用：
  低质心：增加稳定性（不容易翻倒），如车辆/角色
  高质心：容易翻倒，如高塔/堆叠箱子
  偏心质心：旋转时产生偏心振动（如不平衡轮胎）
```

---

## 七、延伸阅读

- 📄 [1.4 物理 Actor 配置详解](./04-physics-actor-setup.md)
- 📄 [1.5 质量与惯性张量](./05-mass-inertia.md)
- 🔗 [Physics - UE 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-in-unreal-engine)
- 📖 推荐书籍：《Physics for Game Developers》David M. Bourg & Bryan Bywalec
