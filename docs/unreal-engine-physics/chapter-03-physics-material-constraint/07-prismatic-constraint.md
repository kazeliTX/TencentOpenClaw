# 3.7 滑动约束（Prismatic Constraint）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、滑动约束原理

```
棱柱约束（Prismatic）= 只允许沿单一轴平移的约束

DOF 配置：
  Linear X：Free 或 Limited（允许沿 X 轴滑动）
  Linear Y/Z：Locked（不能侧移）
  Angular Twist/Swing：Locked（不能旋转）
  
  → 只有 1 个平移 DOF 被释放

现实类比：
  活塞（Piston）：沿轴往复
  电梯：沿 Z 轴上下滑动
  抽屉：沿 X 轴水平滑动
  铁路轨道上的小车
```

---

## 二、电梯约束实现

```
电梯组件设计：
  ElevatorShaft（Static Mesh）：固定，不模拟物理
  ElevatorCabin（Static Mesh）：模拟物理，Mass=500kg
  ElevatorConstraint（PhysicsConstraint）

约束配置：
  连接：ElevatorShaft ←→ ElevatorCabin
  Linear X/Y：Locked
  Linear Z：Limited（Limit = 1000cm，上下各 500cm 行程）
  Angular 全部：Locked
  
  Linear Z Drive：
    Linear Position Drive Z Axis：true
    Target Position Z：1000.f（目标楼层高度）
    Drive Position Strength：800.f（越大电梯越快，但可能振荡）
    Drive Velocity Strength：200.f（阻尼，越大越平稳）
    Max Force：500000.f

电梯楼层控制：
  void CallElevator(float FloorHeight)
  {
      ElevatorConstraint->SetLinearPositionTarget(
          FVector(0, 0, FloorHeight));  // 设置目标高度
  }

  // 检测电梯是否到达楼层
  void Tick(float DeltaTime)
  {
      FVector CurrentPos = ElevatorCabin->GetComponentLocation();
      float HeightDiff = FMath::Abs(CurrentPos.Z - TargetFloorHeight);
      if (HeightDiff < 5.f && bElevatorMoving)
      {
          // 到达！停止驱动
          bElevatorMoving = false;
          ElevatorConstraint->SetLinearDriveParams(0.f, 1000.f, 500000.f);
          // 速度阻尼拉满 → 立即停止
      }
  }
```

---

## 三、活塞约束实现

```
往复活塞（Piston）：
  PistonBody：沿 X 轴往复
  Motor 驱动（速度控制）
  
配置：
  Linear X：Free（无限制，用速度 Drive 控制）
  Linear Y/Z：Locked
  Angular 全部：Locked
  
  Linear Velocity Drive X：true
  Target Velocity X：正值=向前，负值=向后
  Drive Velocity Strength：500.f

曲柄模拟（交替驱动）：
  float PistonPhase = 0.f;
  void Tick(float DeltaTime)
  {
      PistonPhase += DeltaTime * RPM / 60.f * 2.f * PI;
      float TargetVel = FMath::Sin(PistonPhase) * MaxPistonVelocity;
      PistonConstraint->SetLinearVelocityTarget(FVector(TargetVel, 0, 0));
  }
```

---

## 四、延伸阅读

- 📄 [3.8 约束限制详解](./08-joint-limits.md)
- 📄 [3.9 约束驱动器（Motor）](./09-constraint-motor.md)
