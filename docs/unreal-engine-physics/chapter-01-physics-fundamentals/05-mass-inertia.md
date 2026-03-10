# 1.5 质量与惯性张量

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 40 分钟

---

## 一、惯性张量（Inertia Tensor）

```
惯性张量 I 是描述物体抵抗旋转能力的 3x3 矩阵：

      ┌ Ixx  Ixy  Ixz ┐
  I = │ Iyx  Iyy  Iyz │
      └ Izx  Izy  Izz ┘

对角元素（主惯性矩）：
  Ixx = Σ m_i(y_i² + z_i²)   ← 绕 X 轴的转动惯量
  Iyy = Σ m_i(x_i² + z_i²)   ← 绕 Y 轴的转动惯量
  Izz = Σ m_i(x_i² + y_i²)   ← 绕 Z 轴的转动惯量

非对角元素（惯性积）：
  Ixy = -Σ m_i * x_i * y_i
  ...（表示质量分布的对称性）

对于质量均匀分布的常见形状：

  实心球（半径 r，质量 m）：
    Ixx = Iyy = Izz = 2/5 * m * r²

  实心长方体（长 a，宽 b，高 c，质量 m）：
    Ixx = 1/12 * m * (b² + c²)
    Iyy = 1/12 * m * (a² + c²)
    Izz = 1/12 * m * (a² + b²)

  实心圆柱（半径 r，高 h，质量 m，轴为 Z）：
    Ixx = Iyy = 1/12 * m * (3r² + h²)
    Izz = 1/2 * m * r²

  空心球（外径 R，内径 r，质量 m）：
    Ixx = Iyy = Izz = 2/5 * m * (R⁵-r⁵)/(R³-r³)
```

---

## 二、UE 中的惯性张量配置

```
Chaos 自动计算惯性张量：
  根据碰撞形状（凸包/盒/球）的几何形状
  假设质量均匀分布
  计算主惯性矩 (Ixx, Iyy, Izz)

查看当前惯性张量：
  FBodyInstance* BI = Comp->GetBodyInstance();
  FVector InertiaTensor = BI->GetBodyInertiaTensor();
  // 返回 FVector：(Ixx, Iyy, Izz) in kg*cm²

手动缩放惯性张量（InertiaTensorScale）：
  Details → Physics → Inertia Tensor Scale
  默认：(1.0, 1.0, 1.0)
  
  应用场景：
    车辆：降低 Ixx/Iyy（防止过度翻滚）
      InertiaTensorScale = (1.0, 1.0, 3.0) ← Z 轴惯量 3 倍，更难翻车
    
    陀螺：增大旋转轴惯量（更稳定自旋）
      InertiaTensorScale = (0.3, 0.3, 5.0) ← 绕 Z 轴 5 倍惯量

C++ 设置：
  FBodyInstance* BI = Comp->GetBodyInstance();
  BI->InertiaTensorScale = FVector(1.0f, 1.0f, 3.0f);
  BI->UpdateMassProperties(); // 更新生效
```

---

## 三、质心偏移（Center of Mass Offset）

```
为什么需要质心偏移：
  碰撞形状中心 ≠ 实际质量中心时
  自动计算的质心可能不符合物理预期

典型案例：

  案例 1：重型车辆
    问题：默认质心在车身中心（偏高）
    效果：容易翻车，不真实
    修复：COMNudge = FVector(0, 0, -30)  ← 质心下移 30cm
    效果：低重心，更稳定，更真实

  案例 2：吊车吊臂
    问题：吊臂悬伸，质心在后方
    修复：COMNudge = FVector(100, 0, 0)  ← 质心前移
    效果：吊臂更容易前倾（符合实际）

  案例 3：L形物体
    形状一半重一半轻
    修复：通过 COMNudge 手动调整到重的那侧

C++ 设置：
  FBodyInstance* BI = Comp->GetBodyInstance();
  BI->COMNudge = FVector(0.f, 0.f, -20.f);
  BI->UpdateMassProperties();

调试查看质心位置：
  p.Chaos.DebugDraw.ShowCenterOfMass 1
  → 显示每个物理体的质心（红色十字）
```

---

## 四、平行轴定理

```
当质心不在旋转轴上时，用平行轴定理计算惯量：

I_new = I_cm + m * d²

其中：
  I_cm = 绕质心轴的惯量
  m    = 质量
  d    = 质心到旋转轴的距离

应用：复合物体（多个碰撞体组合）
  每个子体先计算自身质心惯量
  再用平行轴定理加到总质心

UE 中复合碰撞体（多个 Simple Collision）：
  每个碰撞形状贡献各自的质量和惯量
  Chaos 自动用平行轴定理合并
  → 形状越分散，总惯量越大（越难旋转）

实例计算（两个球形碰撞体，哑铃形）：
  球 1：m=1kg, r=5cm, 位于 x=-20cm
  球 2：m=1kg, r=5cm, 位于 x=+20cm
  质心：x=0（中间）
  
  每个球绕质心 Z 的惯量：
    I_cm_ball = 2/5*m*r² = 2/5*1*25 = 10 kg*cm²
    I_parallel = I_cm_ball + m*d² = 10 + 1*400 = 410 kg*cm²
  
  总 Izz = 410 + 410 = 820 kg*cm²
  → 远大于单球（10 kg*cm²），旋转难得多
```

---

## 五、质量比对物理交互的影响

```
质量比：重物 vs 轻物的碰撞

质量比过大的问题（如 mass ratio > 100:1）：
  求解器数值不稳定
  轻物体抖动/穿透
  重物体被轻物体"卡住"（数值错误）

建议：
  质量比 < 10:1 → 求解稳定
  质量比 10:1 ~ 100:1 → 可能需要额外阻尼
  质量比 > 100:1 → 用质量覆盖（Mass Overrides）处理

质量覆盖策略：
  超大/超小物体不用真实质量
  而是用"游戏感"质量（接近合理比例）
  
  例：角色（70kg）推动小汽车（200kg，而非1500kg）
    质量比 1:3（合理）vs 实际 1:21（可能抖动）
    游戏感比物理真实感更重要

Maximum Depenetration Velocity（最大去穿透速度）：
  Project Settings → Physics
  默认 100 cm/s
  过小 → 物体缓慢穿出（嵌入很深时解出慢）
  过大 → 去穿透时产生强烈弹开（爆炸感）
```

---

## 六、延伸阅读

- 📄 [1.6 重力与阻尼](./06-gravity-damping.md)
- 🔗 [Physics Bodies - Mass Properties](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-bodies-in-unreal-engine)
- 📖 《Classical Mechanics》- Herbert Goldstein（经典力学标准教材）
