# 1.6 重力与阻尼

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、重力系统

```
UE 默认重力：
  方向：-Z（向下）
  大小：-980 cm/s²（约等于地球 9.8 m/s²）
  注意：UE 使用厘米，所以是 980 而非 9.8

全局重力设置：
  Edit → Project Settings → Physics → Default Gravity Z
  默认值：-980.0
  
  或 WorldSettings → Physics:
    World → Global Gravity Scale：1.0（默认）
    → 设为 0.5：半重力（月球感）
    → 设为 2.0：双倍重力（高重力星球）
    → 设为 0.0：零重力（太空）
    → 设为负值：反重力（向上飘）

单个 Component 的重力控制：
  Details → Physics → Enable Gravity = true/false
  GravityScale（每个 Body 的重力倍数）：
    C++：BI->GravityScale = 0.3f;  // 该物体重力 30%
    → 轻盈飘落的羽毛：GravityScale = 0.1
    → 重型岩石：GravityScale = 1.5
    → 气球（向上）：GravityScale = -0.5（不合理物理但游戏感好）
```

---

## 二、非标准重力方向

```
有时候需要非 -Z 方向的重力（如侧卷轴游戏）：

修改全局重力向量：
  WorldSettings → Physics → Gravity Direction
  默认：(0, 0, -1)
  侧向：(1, 0, 0) 或 (-1, 0, 0)
  
  注意：修改重力方向会影响 CharacterMovementComponent！
  侧卷轴游戏通常不改全局重力，而是用自定义 CMC

重力区域（Physics Volume）：
  APhysicsVolume → bOverrideDefaultPhysicsVolume = true
  重写 GetGravityZ() 返回自定义重力值
  → 只影响进入该区域的角色/物理对象
  
  水下区域：
    GetGravityZ() return -200.0f;  // 水下重力更小
    同时设置较大的 Linear Damping（水阻）
  
  低重力区域（太空站舱室）：
    GetGravityZ() return -50.0f;

C++ 实现重力区域：
  UCLASS()
  class AMyGravityVolume : public APhysicsVolume
  {
      float GetGravityZ() const override { return -200.f; }
  };
```

---

## 三、阻尼详解

```
线性阻尼（Linear Damping）：
  物理意义：流体/空气对平移运动的阻力
  
  数学模型：
    F_damping = -d * v
    其中 d = Linear Damping，v = 当前速度
    
  在 Chaos 中的积分：
    v(t+Δt) = v(t) * max(0, 1 - d*Δt)
    （近似指数衰减，避免负值）
  
  终端速度（Terminal Velocity）：
    当重力 = 阻力时，物体达到终端速度
    v_terminal = (m * g) / d
    
    例：m=1kg, g=980cm/s², d=1.0
        v_terminal = 980 cm/s = 9.8 m/s（约自由落体终端）

角阻尼（Angular Damping）：
  物理意义：旋转运动的阻力（摩擦+空气阻力）
  
  数学模型：
    τ_damping = -d_ang * ω
    
  在 Chaos 中：
    ω(t+Δt) = ω(t) * max(0, 1 - d_ang*Δt)

常见物体阻尼参考值：
  ────────────────────────────────────────
  物体          Linear    Angular
  ────────────────────────────────────────
  太空漂浮物    0.0       0.0
  普通金属球    0.1       0.1
  充气球        0.5       0.3
  木块（空气）  0.1       0.5
  水中浮球      5.0       3.0
  沙中物体      20.0      15.0
  角色（垂直）  0.0       10.0（防止角色翻滚）
  ────────────────────────────────────────
```

---

## 四、空气阻力模拟（高精度）

```
真实空气阻力公式：
  F_drag = 0.5 * ρ * Cd * A * v²
  
  ρ  = 空气密度 (1.225 kg/m³ at 海平面)
  Cd = 阻力系数 (球体≈0.47, 流线形≈0.04)
  A  = 截面积 (m²)
  v  = 速度 (m/s)

UE 内建的 Linear Damping 是线性近似（F=-dv）
更真实的空气阻力是平方项（F=-kv²）

在 SubstepTick 中实现精确空气阻力：
  void PhysicsSubstep(float DeltaTime, FBodyInstance* BodyInst)
  {
      FVector Velocity = BodyInst->GetUnrealWorldVelocity();
      float Speed = Velocity.Size();
      if (Speed > 0.f)
      {
          // F_drag = 0.5 * rho * Cd * A * v²（单位换算到 UE cm/s）
          const float RhoAir  = 0.0000012f; // kg/cm³
          const float Cd      = 0.47f;       // 球体
          const float Area    = 78.5f;       // cm²（半径5cm的球）
          float ForceMag = 0.5f * RhoAir * Cd * Area * Speed * Speed;
          FVector DragForce = -Velocity.GetSafeNormal() * ForceMag;
          BodyInst->AddForce(DragForce, false, false);
      }
  }
```

---

## 五、延伸阅读

- 📄 [1.7 物理帧更新机制](./07-physics-tick.md)
- 🔗 [Physics Volumes](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-volumes-in-unreal-engine)
