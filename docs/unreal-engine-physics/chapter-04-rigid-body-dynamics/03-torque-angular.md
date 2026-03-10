# 4.3 力矩与角速度

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、力矩（Torque）基础

```
力矩 τ（Torque）= r × F
  r = 从质心到施力点的向量
  F = 施加的力
  × = 叉积（Cross Product）
  
  大小：|τ| = |r| * |F| * sin(θ)
  θ = r 与 F 的夹角
  力矩最大：r ⊥ F（垂直时力臂最大）
  力矩为零：F 过质心（推/拉质心不产生旋转）

方向（右手定则）：
  右手握拳，四指从 r 转向 F
  拇指方向 = 力矩方向（旋转轴）

加速旋转公式：
  τ = I * α
  I = 惯性张量
  α = 角加速度

UE 中施加力矩：
  Mesh->AddTorqueInDegrees(FVector(0, 0, 1000.f));  // 绕 Z 轴旋转
  Mesh->AddTorqueInRadians(FVector(0, 0, 17.45f));  // 同上
  
  bAccelChange=true：
    直接施加角加速度（质量/惯性无关）
  false（默认）：
    施加力矩（惯性大的物体旋转慢）
```

---

## 二、AddImpulseAtLocation 产生的旋转

```
在非质心点施加冲量时，同时产生线冲量和角冲量：

Δv_linear  = J / m                              （线速度变化）
Δω_angular = I⁻¹ * (r × J)                      （角速度变化）

示例：子弹打在角色侧面（非质心）：
  质心在腰部（Z=90cm），子弹打在肩部（Z=160cm）
  r = 肩部 - 腰部 = (0, 0, 70cm)（局部坐标）
  J = 子弹冲量 = (-1000, 0, 0)（向后）
  
  τ = r × J = (0,0,70) × (-1000,0,0)
            = (0*0 - 70*0, 70*(-1000) - 0*0, 0*0 - 0*(-1000))
            = (0, -70000, 0)
  → 绕 Y 轴产生力矩（角色向前倒）

C++ 验证：
  // 子弹命中角色肩部
  FVector BulletDir = FVector(-1, 0, 0);  // 子弹方向
  float BulletImpulseMag = 30000.f;
  
  // AddImpulseAtLocation 自动计算并施加角冲量
  TargetComp->AddImpulseAtLocation(
      BulletDir * BulletImpulseMag,
      Hit.ImpactPoint);
  
  // 等价于手动分离：
  TargetComp->AddImpulse(BulletDir * BulletImpulseMag); // 线
  FVector r = Hit.ImpactPoint - TargetComp->GetCenterOfMass();
  FVector AngImpulse = FVector::CrossProduct(r, BulletDir * BulletImpulseMag);
  TargetComp->AddAngularImpulseInRadians(AngImpulse);  // 角
```

---

## 三、旋转控制应用

```
陀螺仪效果（保持旋转轴稳定）：
  高速旋转的物体具有角动量，抵抗方向改变
  UE 中 Chaos 自动处理陀螺效应
  
  陀螺进动（Gyroscopic Precession）：
    旋转轴在外力矩作用下缓慢偏转
    τ_precession ⊥ ω（角速度）且 ⊥ τ（外力矩）
    
  注意：Chaos 5.x 支持陀螺效应模拟
    需要 p.Chaos.Gyroscopic 1（开启，默认关闭以节省性能）

弹道旋转稳定（子弹/箭矢）：
  子弹高速旋转 → 陀螺效应保持弹尖朝前
  
  // 初始化子弹旋转
  float BulletSpinRPS = 10.f;  // 10 转/秒
  float SpinRadPerSec = BulletSpinRPS * 2.f * PI;
  BulletMesh->SetPhysicsAngularVelocityInRadians(
      GetActorForwardVector() * SpinRadPerSec);

车辆翻滚控制：
  防止汽车过度翻滚 → 施加反向稳定力矩
  
  void Tick(float Dt)
  {
      FVector AngVel = CarMesh->GetPhysicsAngularVelocityInRadians();
      // 滚转分量（绕前进轴旋转）
      float RollRate = FVector::DotProduct(AngVel, GetActorForwardVector());
      if (FMath::Abs(RollRate) > 1.f)  // 超过 1 rad/s 才矫正
      {
          FVector StabilizeTorque = -GetActorForwardVector() * RollRate * 50000.f;
          CarMesh->AddTorqueInRadians(StabilizeTorque, NAME_None, false);
      }
  }
```

---

## 四、延伸阅读

- 📄 [4.4 爆炸力场](./04-explosion-force.md)
- 📄 [4.6 点施力详解](./06-force-at-location.md)
