# 9.5 绳索附加与断裂

> **难度**：⭐⭐⭐⭐⭐

## 一、约束断裂配置

```cpp
// 绳索约束断裂配置
UPhysicsConstraintComponent* PC = ...;
// 线性断裂力（拉伸超过此力时断裂）
PC->ConstraintInstance.ProfileInstance.LinearLimit.bSoftConstraint = false;
// 通过 API 设置断裂阈值
PC->ConstraintInstance.SetLinearBreakable(true, 5000.f);  // 5000N 时断裂
PC->ConstraintInstance.SetAngularBreakable(true, 3000.f); // 3000N·m 时断裂

// 监听断裂事件（组件上）
PC->OnConstraintBroken.AddDynamic(this, &AMyActor::OnRopeBroken);
```

## 二、玩家抓绳/挂绳

```
玩家抓住绳索：
  1. 重叠检测命中绳段 Body
  2. 创建 PhysicsConstraint 将玩家手部 attach 到该绳段
  3. 禁用角色的 CharacterMovement → 切换到 Flying 模式
  4. 玩家输入 → 向约束施加线性力（沿绳方向爬升）
  
放开绳索：
  5. 销毁抓绳约束
  6. 恢复 CharacterMovement
  
  关键：创建的约束只允许绳方向有力（Limit 其他轴）
       防止玩家悬挂时身体乱转
```

## 三、延伸阅读

- 📄 [9.6 软体碰撞](./06-soft-body-collision.md)
