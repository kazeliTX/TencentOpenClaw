# 4.1 力的种类与施加方式

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 35 分钟

---

## 一、UE 力 API 汇总

```
UPrimitiveComponent 提供的力接口：

持续力（Force）— 每帧累积：
  AddForce(Force, BoneName, bAccelChange)
    Force：力向量（Newton = kg*cm/s²）
    bAccelChange = false：力（F=ma，质量影响加速度）
    bAccelChange = true ：加速度变化（质量无关，直接改变加速度）
    → 用 true 可以"忽略质量"，让不同质量物体同样加速

  AddForceAtLocation(Force, Location, BoneName)
    在世界坐标点施加力（同时产生力矩！）

  AddLocalForce(Force, BoneName, bAccelChange)
    在组件局部坐标系施加力（跟随旋转）

  AddLocalForceAtLocation(Force, Location, BoneName)

冲量（Impulse）— 瞬时：
  AddImpulse(Impulse, BoneName, bVelChange)
    bVelChange = false：冲量 J（Δv = J/m）
    bVelChange = true ：速度变化（质量无关，直接设 Δv）

  AddImpulseAtLocation(Impulse, Location, BoneName)
    在指定点施加冲量（同时产生角冲量）

  AddLocalImpulse(Impulse, BoneName, bVelChange)

力矩（Torque）：
  AddTorqueInDegrees(Torque, BoneName, bAccelChange)
  AddTorqueInRadians(Torque, BoneName, bAccelChange)

角冲量（Angular Impulse）：
  AddAngularImpulseInDegrees(Impulse, BoneName, bVelChange)
  AddAngularImpulseInRadians(Impulse, BoneName, bVelChange)
```

---

## 二、bAccelChange / bVelChange 的实际意义

```
场景：三个箱子，质量分别 1kg / 10kg / 100kg
目标：同时对三个箱子施加"相同加速度" = 500 cm/s²

方案 A：AddForce(F=500)，bAccelChange=false
  1kg 箱：加速度 = 500/1  = 500 cm/s²  ✅
  10kg 箱：加速度 = 500/10 = 50 cm/s²  ❌（轻箱飞起，重箱几乎不动）

方案 B：AddForce(F=500)，bAccelChange=true（直接施加加速度）
  Chaos 内部：F_actual = 500 * mass
  1kg 箱：施加 500 N，加速度 = 500 cm/s²  ✅
  10kg 箱：施加 5000 N，加速度 = 500 cm/s² ✅
  100kg 箱：施加 50000 N，加速度 = 500 cm/s² ✅
  → 质量无关，所有物体同样加速

使用场景：
  bAccelChange=true：
    重力（所有物体自由落体一样）
    风场（对所有物体施加相同加速度）
    磁场（均匀吸引力）
  bAccelChange=false（默认）：
    发动机推力（相同力，重车加速慢）
    爆炸力（相同冲量，轻物飞更远）
    碰撞力（基于质量的物理真实效果）
```

---

## 三、力的叠加与帧率无关性

```
力的帧率依赖问题：
  在 Tick 中每帧调用 AddForce(F)
  60 FPS：力作用 Δt=16.7ms → Δv = F/m * 0.016s
  30 FPS：力作用 Δt=33.3ms → Δv = F/m * 0.033s（速度变化翻倍！）
  
  结果：30 FPS 时加速更快，物理行为不一致

帧率无关方案 1：乘以 DeltaTime
  AddForce(F * DeltaTime)  ❌ 错误！
  AddForce 内部已经会乘以步长，再乘一次会双重计算
  
帧率无关方案 2（正确）：使用 Substepping
  在 SubstepTick 中施加力
  SubstepTick 的 DeltaTime = 固定步长（如 16.7ms）
  → 每步长施加相同的力，帧率无关 ✅
  
帧率无关方案 3：用冲量代替力
  只需要"一次性效果"→ 用 AddImpulse（不依赖帧率）
  
持续力（如火箭推进）→ 必须用 SubstepTick
瞬时效果（跳跃/爆炸）→ 用 AddImpulse
```

---

## 四、常见力的大小参考

```
UE 默认单位：厘米/秒（cm/s），质量 kg

公式换算：
  F = m * a  （N = kg * m/s²，UE 用 cm/s² 所以 F 单位为 kg*cm/s²）
  
  1 N = 1 kg*m/s² = 100 kg*cm/s²
  所以 UE 中施加 1 N 的力 = AddForce(100)（如果质量1kg）

常见参考值（UE 单位）：
  让 1kg 物体以 1g 加速：F = 1 * 980 = 980
  人推开 10kg 箱子（1m/s 的初速）：Impulse ≈ 10 * 100 = 1000
  爆炸中心（半径200cm，10kg物体飞3m/s）：
    Impulse ≈ 10 * 300 = 3000（在中心）
  角色跳跃冲量（70kg角色跳1m高）：
    v_jump = sqrt(2*g*h) = sqrt(2*980*100) ≈ 443 cm/s
    Impulse_z = 70 * 443 ≈ 31010 kg*cm/s
  车辆引擎推力（1500kg, 0→100kph/5s）：
    a = (100*100/3.6) / 5 ≈ 556 cm/s²
    F = 1500 * 556 ≈ 834000
```

---

## 五、延伸阅读

- 📄 [4.2 冲量与速度控制](./02-impulse-velocity.md)
- 📄 [代码示例：力与冲量](./code/01_force_impulse.cpp)
- 🔗 [UPrimitiveComponent Physics API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Components/UPrimitiveComponent)
