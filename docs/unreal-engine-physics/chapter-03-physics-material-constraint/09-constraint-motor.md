# 3.9 约束驱动器（Constraint Motor / Drive）

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 35 分钟

---

## 一、Drive 系统概述

```
Drive = 约束内置的主动驱动力/扭矩
  不需要外部 AddForce，约束自己产生力推向目标

两种 Drive 模式：
  Position Drive（位置驱动）：
    目标 = 特定位置/旋转
    弹簧 + 阻尼 → 驱向目标位置
    用途：电梯归位、门自动关、关节归位
  
  Velocity Drive（速度驱动）：
    目标 = 特定速度/角速度
    阻尼 → 使当前速度接近目标速度
    用途：电机（恒速旋转）、输送带、风扇

参数：
  Drive Position Strength（Position Drive）：弹簧刚度 k
  Drive Velocity Strength（两者都有）：阻尼 c
  Maximum Force：驱动力/扭矩上限
  Target Position / Target Velocity：驱动目标
```

---

## 二、Angular Drive 模式

```
Angular Drive Mode：
  SLERP Drive（默认）：
    用球面线性插值驱动朝向
    用一组 PD 参数控制所有三个旋转轴
    简单，适合大多数情况
  
  Twist and Swing Drive：
    Twist 轴和 Swing 轴分别有独立的 Drive 参数
    适合需要精细控制（如机械手臂：旋转轴和摆动轴刚度不同）

SLERP Drive 常用参数示例：

  // 门自动关闭
  Constraint->SetAngularDriveMode(EAngularDriveMode::SLERP);
  Constraint->SetOrientationDriveSLERP(true);  // 启用位置驱动
  Constraint->SetAngularOrientationTarget(FQuat::Identity);  // 目标：关闭角度
  Constraint->SetAngularDriveParams(
      200.f,   // Stiffness（越大关门越快）
      50.f,    // Damping（越大越平稳，不振荡）
      2000.f   // MaxForce
  );

  // 恒速旋转（电机）
  Constraint->SetAngularDriveMode(EAngularDriveMode::SLERP);
  Constraint->SetAngularVelocityDriveSLERP(true);  // 启用速度驱动
  Constraint->SetAngularVelocityTarget(FVector(0, 0, 5.f));  // 5 rad/s 绕 Z 轴
  Constraint->SetAngularDriveParams(
      0.f,      // Stiffness = 0（纯速度驱动，不需要位置弹簧）
      500.f,    // Damping（提供驱动力）
      100000.f  // MaxForce
  );
```

---

## 三、Linear Drive 常用场景

```
// 悬停平台（保持在指定高度）
void AHoverPlatform::BeginPlay()
{
    Super::BeginPlay();
    // Linear Z：Free + Position Drive
    PhysConstraint->SetLinearPositionDrive(false, false, true); // Z轴
    PhysConstraint->SetLinearPositionTarget(FVector(0, 0, TargetHoverHeight));
    PhysConstraint->SetLinearDriveParams(
        800.f,     // Stiffness（越大悬浮越稳）
        100.f,     // Damping
        200000.f   // MaxForce
    );
}

// 滑动抽屉（推开后自动关闭）
void ADrawer::SetOpen(bool bOpen)
{
    float TargetX = bOpen ? 60.f : 0.f;  // 打开 60cm，关闭 0
    PhysConstraint->SetLinearPositionTarget(FVector(TargetX, 0, 0));
    PhysConstraint->SetLinearPositionDrive(true, false, false);
    PhysConstraint->SetLinearDriveParams(
        300.f,   // Stiffness
        80.f,    // Damping
        50000.f  // MaxForce
    );
}
```

---

## 四、Drive 参数调优速查表

```
目标效果          Position Strength   Velocity Strength   Max Force
────────────────────────────────────────────────────────────────────
快速/硬回弹       1000                50                  100000
标准回弹          300                 50                  50000
缓慢/柔和回弹     50                  20                  10000
恒速电机          0                   500                 100000
制动/停车         0                   5000                500000
悬浮平台          800                 100                 200000
────────────────────────────────────────────────────────────────────
注：Max Force 需 > Position/Velocity Strength * 位移/速度量级
    否则驱动力被截断，无法到达目标
```

---

## 五、延伸阅读

- 📄 [3.10 绳索与链条](./10-rope-chain.md)
- 📄 [代码示例：铰链门与电机](./code/02_constraint_door.cpp)
