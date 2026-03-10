# 4.2 冲量与速度控制

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、冲量详解

```
冲量（Impulse）J：
  J = F * Δt（力 × 时间）
  等效于动量变化：J = m * Δv
  → Δv = J / m

AddImpulse vs AddForce：
  AddForce(F)：每帧 Δv += F/m * Δt（积分）
  AddImpulse(J)：立即 Δv += J/m（单次）
  
  AddImpulse(F * Δt) ≈ AddForce(F) 作用一帧（数值上等价）

bVelChange 参数：
  false（默认）：Δv = J / mass（质量影响）
  true：         Δv = J（直接设速度增量，不管质量）
  
  使用 true 的场景：
    传送带/拾取物（强制给速度，不管物体多重）
    跳跃（角色需要固定起跳速度，不管重量）
    弹弓发射（固定初速，不管弹丸质量）
```

---

## 二、速度直接设置

```
SetPhysicsLinearVelocity(Velocity, bAddToCurrent)
  bAddToCurrent = false：直接设为此速度（覆盖当前速度）
  bAddToCurrent = true ：在当前速度上叠加

SetPhysicsAngularVelocityInDegrees(Velocity)
SetPhysicsAngularVelocityInRadians(Velocity)

获取当前速度：
  GetPhysicsLinearVelocity()              → FVector (cm/s)
  GetPhysicsLinearVelocityAtPoint(Point)  → 某点的线速度（含旋转贡献）
  GetPhysicsAngularVelocityInDegrees()    → FVector (deg/s)
  GetPhysicsAngularVelocityInRadians()    → FVector (rad/s)

速度限制（运行时）：
  // 限制最大速度（防止数值爆炸）
  FVector Vel = Mesh->GetPhysicsLinearVelocity();
  const float MaxSpeed = 2000.f;
  if (Vel.SizeSquared() > MaxSpeed * MaxSpeed)
      Mesh->SetPhysicsLinearVelocity(Vel.GetSafeNormal() * MaxSpeed);

  // 物理设置中的全局上限
  // Project Settings → Physics → Max Angular Velocity：3600 deg/s
```

---

## 三、冲量应用场景

```
跳跃（Jump Impulse）：
  float JumpSpeed = 800.f;
  FVector JumpImpulse = FVector(0, 0, JumpSpeed);
  Mesh->AddImpulse(JumpImpulse, NAME_None, true); // bVelChange=true
  // → 不管物体质量，直接获得 800cm/s 向上速度

爆炸击飞（见 4.4 爆炸力场详细示例）：
  FVector Dir = (TargetLoc - ExplosionLoc).GetSafeNormal();
  float Dist = FVector::Distance(TargetLoc, ExplosionLoc);
  float FallOff = FMath::Clamp(1.f - Dist/Radius, 0.f, 1.f);
  Comp->AddImpulse(Dir * BaseImpulse * FallOff, NAME_None, false);

乒乓球弹射（精确弹性碰撞）：
  // 在 Hit 事件中手动计算弹性
  FVector V = Ball->GetPhysicsLinearVelocity();
  FVector Normal = Hit.ImpactNormal;
  FVector Reflected = V - 2.f * FVector::DotProduct(V, Normal) * Normal;
  Reflected *= Restitution; // 弹性系数衰减
  Ball->SetPhysicsLinearVelocity(Reflected);
  // 这比依赖 Chaos 自动弹性更精确可控

击退效果（角色被击中）：
  FVector KnockDir = (CharLoc - AttackerLoc).GetSafeNormal();
  KnockDir.Z = 0.5f; // 向上分量（飞起感）
  KnockDir.Normalize();
  CapsuleComp->AddImpulse(KnockDir * 200000.f);
  // 注意：CharacterMovementComponent 会抵消部分冲量
  // 建议：短暂禁用 CMC 后再施加冲量
```

---

## 四、速度插值（平滑控制）

```
有时需要物理物体平滑移动到目标速度（不是瞬间）：

// 每帧在 SubstepTick 中逐渐调整速度
void SubstepTick(float Dt, FBodyInstance* BI)
{
    FVector CurrentVel = BI->GetUnrealWorldVelocity();
    FVector TargetVel  = FVector(500, 0, 0);  // 目标速度
    
    // 插值（指数平滑）
    float Alpha = 1.f - FMath::Exp(-5.f * Dt); // 时间常数 = 0.2s
    FVector NewVel = FMath::Lerp(CurrentVel, TargetVel, Alpha);
    
    // 通过力来实现（不直接设速度，保留物理交互）
    FVector DeltaV = NewVel - CurrentVel;
    FVector Force = DeltaV * BI->GetBodyMass() / Dt;
    BI->AddForce(Force, false, false);
}
```

---

## 五、延伸阅读

- 📄 [4.3 力矩与角速度](./03-torque-angular.md)
- 📄 [4.4 爆炸力场](./04-explosion-force.md)
