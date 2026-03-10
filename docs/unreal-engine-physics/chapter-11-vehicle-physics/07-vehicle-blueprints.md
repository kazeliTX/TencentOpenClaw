# 11.7 蓝图车辆设置完整流程

> **难度**：⭐⭐⭐⭐☆

## 完整创建步骤

```
Step 1：准备 SkeletalMesh
  车身为 Mesh 主体
  每个车轮为单独骨骼（wheel_FL/FR/RL/RR）
  确保车轮骨骼朝向：旋转轴为 Y 轴（侧向）

Step 2：创建 PhysicsAsset
  SkeletalMesh → Create PhysicsAsset
  车身：一个大 Box Body（覆盖主体）
  每个车轮骨骼：小 Sphere/Cylinder Body
  设置车身 Body 为 Simulate Physics

Step 3：创建 Wheel BP
  新建 BP → 父类：ChaosVehicleWheel
  配置半径/宽度/摩擦/悬挂参数（4个轮各一个BP）

Step 4：创建 VehicleAnimBP
  父类：VehicleAnimationBlueprint
  AnimGraph：Mesh Space Refs → Wheel Handler → Output Pose
  Wheel Handler：绑定对应的 VehicleMovementComponent

Step 5：创建 VehiclePawn BP
  父类：ChaosWheeledVehiclePawn
  添加 SkeletalMeshComponent → 设置 Mesh + AnimBP
  ChaosWheeledVehicleMovement（自动添加）
  配置 WheelSetups（4个轮 + 对应骨骼名）
  配置发动机/变速箱/差速器参数

Step 6：输入绑定（Input Action）
  油门（W/RT）→ SetThrottleInput(1.0)
  刹车（S/LT）→ SetBrakeInput(1.0)
  转向（AD/LS）→ SetSteeringInput(-1~1)
  手刹（Space）→ SetHandbrakeInput(true/false)
```
