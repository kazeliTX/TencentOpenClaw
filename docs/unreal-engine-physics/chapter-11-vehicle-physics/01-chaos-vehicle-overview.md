# 11.1 Chaos Vehicle 总览

> **难度**：⭐⭐⭐⭐☆

## 一、Chaos Vehicle vs PhysX Vehicle

```
UE4 PhysX Vehicle → UE5 Chaos Vehicle（完全重写）

主要变化：
  求解器：PhysX → Chaos（原生集成）
  轮胎模型：Pacejka 魔幻公式（更真实）
  悬挂：独立弹簧/阻尼（每轮可独立配置）
  驱动模式：FWD/RWD/AWD/NWD（自由配置每轮驱动力）
  差速器：开放式/限滑差速（LSD）

启用插件：
  Edit → Plugins → ChaosVehiclesPlugin ✓
  重启编辑器

基础类：
  UChaosVehicleMovementComponent（核心）
  UChaosWheeledVehicleMovementComponent（带轮车辆）
  AChaosWheeledVehiclePawn（基础车辆 Pawn）

Vehicle 创建步骤：
  1. 准备车身 SkeletalMesh（骨骼对应车轮位置）
  2. 创建 PhysicsAsset（车身 + 每个车轮的 Body）
  3. 创建 VehicleAnimationBP（连接车轮旋转/转向）
  4. 创建 VehiclePawn BP，添加 ChaosWheeledVehicleMovement
  5. 配置轮胎/悬挂/发动机参数
  6. 添加输入绑定（油门/刹车/转向）
```

## 二、坐标系约定

```
Chaos Vehicle 坐标系：
  X+ = 车辆前方（前进方向）
  Y+ = 车辆右方
  Z+ = 车辆上方

轮子位置（WheelSetup.BoneName）：
  必须与 SkeletalMesh 中的骨骼名一致
  通常：wheel_FL / wheel_FR / wheel_RL / wheel_RR
  (Front Left / Front Right / Rear Left / Rear Right)
```
