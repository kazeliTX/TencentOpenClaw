# 10.7 程序化动画

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、程序化动画的概念

```
程序化动画 = 用代码/Rig Graph 在运行时生成动画
  不需要预制动画序列
  能够实时响应环境和角色状态

典型程序化动画：
  呼吸动画         ← 按时间周期上下浮动脊椎/胸腔
  眨眼动画         ← 随机间隔触发眼睑闭合
  头部追踪         ← 头部骨骼朝向目标（Look At）
  脚步地面适配     ← IK 实时适配地面高度（已在第五章讲解）
  尾巴/耳朵摆动   ← AnimDynamics（见第七章）
  程序化脊椎弯曲   ← 根据移动速度动态弯曲脊椎
```

---

## 二、呼吸动画（Rig Graph 实现）

```
在 Rig Graph 中实现程序化呼吸：

Rig Graph 节点（Forward Solve）：

// 呼吸周期
Time         = GetCurrentTime()
BreathCycle  = Sin(Time × BreathRate × 2π)
              ← BreathRate：呼吸频率（次/秒，0.3=每3秒一次）

// 胸腔骨骼（spine_02/spine_03）
ChestExpand  = BreathCycle × BreathStrength
              ← BreathStrength：呼吸幅度（旋转角度，如 2°）

// 叠加到基础骨骼
SpineRot = GetBoneTransform(spine_02)
SpineRot.Rotation += FRotator(ChestExpand, 0, 0)
SetBoneTransform(spine_02, SpineRot, Local)

// 肩部辅助（呼吸时肩部微微上下）
ShoulderOffset = BreathCycle × 0.3
SetBoneTranslation(clavicle_l, Base + FVector(0, 0, ShoulderOffset), Local)
SetBoneTranslation(clavicle_r, Base + FVector(0, 0, ShoulderOffset), Local)

参数（成员变量，供 AnimBP 设置）：
  BreathRate:     0.3    （站立时，疲劳时增大到 0.5）
  BreathStrength: 2.0    （待机时，奔跑后增大到 4.0）
```

---

## 三、头部朝向目标（Look At）

```
Look At 节点实现角色看向目标：

节点配置：
  Bone:         head           ← 要旋转的骨骼
  Look At Axis: X              ← 骨骼的"前方"轴
  Target:       LookAtPosition ← 注视目标位置（全局空间）
  
  Clamp Cone Angle: 60°        ← 最大旋转角度（超过就不转了）
  Interpolation Speed: 10.0    ← 插值速度（平滑转头）

分层实现（头+颈+脊椎上部联动）：
  Head:    50% 权重的旋转
  neck:    30% 权重的旋转
  spine_03: 20% 权重的旋转
  → 自然的全身朝向感

在 AnimBP 中更新 LookAtPosition：
  LookAtPosition = EnemyActor->GetActorLocation()
```

---

## 四、程序化脊椎弯曲

```
根据速度和加速度动态弯曲脊椎（跑步前倾效果）：

// Rig Graph 计算
LeanAngle = Velocity.X × LeanScale
           + Acceleration.X × AccelLeanScale

// 分配到多个脊椎骨骼（越高的骨骼弯曲越多）
SpineWeights = [0.1, 0.2, 0.3, 0.4]  // spine_01 到 spine_04

For spine_01 ~ spine_04:
  BendAmount = LeanAngle × SpineWeights[i]
  SetBoneRotation(Spine[i], FRotator(BendAmount, 0, 0), Local)

配合 Inertialization：
  当倾斜值突然变化时（如从跑步到停止）
  用 Inertialization 节点平滑过渡
  避免脊椎突然跳变
```

---

## 五、延伸阅读

- 📄 [代码示例：程序化脊椎](./code/02_procedural_spine.cpp)
- 🔗 [Control Rig 程序化动画](https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-animation-in-unreal-engine)
