# 3.5 铰链约束（Hinge Constraint）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、铰链约束原理

```
铰链（Hinge）= 只允许绕单一轴旋转的约束
现实类比：门铰链、轮子轴承、肘关节、翻盖

DOF 配置：
  Linear X/Y/Z：Locked（不允许平移）
  Angular Twist：Free  （允许绕 Constraint X 轴旋转）
  Angular Swing 1/2：Locked（不允许其他角度摆动）

  → 只有 1 个旋转 DOF 被释放

角度限制（有限铰链）：
  Angular Twist Motion = Limited（而非 Free）
  Twist Limit Angle：如 ±90°（门只能开 90 度）
  软限制 vs 硬限制：
    软限制（Soft Limit）：有弹性，超出后被弹回（弹簧感）
    硬限制（Hard Limit）：立即阻止（刚性感）
```

---

## 二、门的完整实现

```
蓝图配置方案：

1. 创建门 Actor（Door_BP）：
   Components：
     DoorFrame（Static Mesh）- Root，设为 Simulate Physics = false（固定）
     DoorPanel（Static Mesh）- Simulate Physics = true，mass=20kg
     HingeConstraint（PhysicsConstraintComponent）

2. HingeConstraint 设置：
   Component Name 1：DoorFrame
   Component Name 2：DoorPanel
   
   Linear X/Y/Z：Locked
   Angular Twist Motion：Limited
   Twist Limit Angle：100.0（门可开到 100°）
   Soft Constraint：true（开到极限时有弹力缓冲）
   
   Angular Swing 1 Motion：Locked
   Angular Swing 2 Motion：Locked
   
   Constraint Frame：调整 X 轴对准铰链方向（Z 轴/门的旋转轴）

3. 门的推力触发（角色推门）：
   在 DoorPanel 的 OnComponentBeginOverlap 中：
     DoorPanel->AddImpulse(PushDirection * 30000.f);

4. 自动关门（弹簧驱动）：
   HingeConstraint → Angular Drive（Mode: Orientation Drive）
   Target Orientation：（门关闭时的旋转，通常为 0）
   Drive Position Strength：100.f（弹簧刚度，越大关门越快）
   Drive Velocity Strength：50.f（阻尼，防止过冲振荡）
   Maximum Force：500.f
```

---

## 三、车轮铰链（轮子轴承）

```
车轮需要完全自由旋转（无限制）+ 摩擦：

配置：
  Linear X/Y/Z：Locked（轮子不脱离车轴）
  Angular Twist：Free（轮子自由旋转）
  Angular Swing 1/2：Locked（不允许侧倾）

车轮驱动（Motor 模式）：
  Angular Drive Mode：Velocity Drive（速度驱动）
  Target Angular Velocity：指定转速（rad/s）
  Drive Velocity Strength：发动机扭矩（越大加速越快）
  Maximum Force：发动机最大扭矩
  
  // 油门踩下 → 增大 Target Angular Velocity
  void EngineAccelerate(float Throttle)
  {
      float TargetRPM = MaxRPM * Throttle;
      float TargetRadPerSec = FMath::DegreesToRadians(TargetRPM * 6.f); // 1 RPM = 6 deg/s
      WheelConstraint->SetAngularVelocityTarget(FVector(TargetRadPerSec, 0, 0));
  }
  
  // 刹车 → 停止驱动 + 增大阻尼
  void Brake()
  {
      WheelConstraint->SetAngularVelocityTarget(FVector::ZeroVector);
      WheelConstraint->SetAngularDriveParams(0.f, 5000.f, 50000.f);
      // 阻尼拉满 → 轮子迅速停止
  }

注意：Chaos Vehicle（第11章）使用专门的悬挂约束系统，
     不需要手动创建轮子铰链约束。
     手动铰链主要用于自定义载具或机械装置。
```

---

## 四、铰链约束调试

```
常见问题：

问题：门绕错误轴旋转（如水平旋转而不是垂直旋转）
原因：Constraint X 轴方向错误
修复：旋转 PhysicsConstraintComponent，使其 X 轴对准铰链方向
     可在编辑器中用 Local Axis 显示确认

问题：门在铰链处抖动
原因：角阻尼不足 + 软限制弹力过强
修复：
  DoorPanel->SetAngularDamping(5.0f);
  减小 Drive Position Strength（如从 500 → 100）

问题：两个刚体约束后分离（没有真正连接）
原因：Component Name 设置错误，约束找不到目标组件
修复：检查 Component 1/2 Name 是否与 Mesh Component 名称完全一致（大小写敏感！）

可视化：
  show constraints   ← 显示约束轴（编辑器调试）
  p.Chaos.DebugDraw.ShowConstraints 1  ← 运行时显示
```

---

## 五、延伸阅读

- 📄 [3.6 弹簧约束](./06-spring-constraint.md)
- 📄 [3.7 滑动约束](./07-prismatic-constraint.md)
- 📄 [代码示例：铰链门实现](./code/02_constraint_door.cpp)
- 🔗 [Physics Constraints User Guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-constraints-user-guide-in-unreal-engine)
