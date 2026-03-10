# 11.5 轮胎摩擦与打滑

> **难度**：⭐⭐⭐⭐⭐

## 一、Pacejka 轮胎模型

```
Chaos Vehicle 使用 Pacejka "Magic Formula"（魔幻公式）：

  F(α) = D × sin(C × arctan(B×α - E×(B×α - arctan(B×α))))
  
  其中：
    α = 打滑角（Side Slip）或纵向滑移率（Longitudinal Slip）
    B = 刚性因子（Stiffness Factor）
    C = 形状因子（Shape Factor）
    D = 峰值系数（Peak Factor）= μ × FN（摩擦系数 × 法向力）
    E = 曲率因子（Curvature Factor）
    
  UE5 简化参数（UChaosVehicleWheel 对应）：
    FrictionForceMultiplier = D（峰值摩擦）
    SlipThreshold：开始打滑的阈值
    SkidThreshold：完全打滑（轮胎吱吱声）阈值
```

## 二、漂移调参

```
实现漂移（Drift）的参数配置：

后轮：
  bAffectedByHandbrake = true（手刹锁后轮）
  FrictionForceMultiplier = 1.5~2.0（不要太高，便于打滑）
  
前轮：
  MaxSteerAngle = 35~45°（更大转向角 = 更容易控制漂移）
  FrictionForceMultiplier = 2.5（前轮抓地力强，控制方向）
  
发动机：
  高扭矩 + 后驱（RWD）= 油门过多时后轮打滑
  
物理材质（路面）：
  路面 Friction = 0.7~0.8（正常沥青）
  湿滑路面 = 0.4（更容易漂移）
  
手感调整：
  增大 AngularDamping → 减少无规律旋转（更可控）
  增大 LinearDamping → 减少翻车（稳定性↑）
```
