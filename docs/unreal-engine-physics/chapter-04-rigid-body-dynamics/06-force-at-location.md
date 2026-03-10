# 4.6 点施力（AddForceAtLocation）

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 30 分钟

---

## 一、点施力的物理效果

```
AddForceAtLocation(F, Location)：
  等价于：
    AddForce(F)                              ← 线力（平移）
    AddTorque(r × F)                         ← 力矩（旋转）
  其中 r = Location - CenterOfMass

图示：
  质心 ●───r────> 施力点 X→F
       ↑
  产生力矩 τ = r × F（使物体旋转）

施力点对效果的影响：
  在质心施力：只产生平移，无旋转
  在质心以上施力（开门的上部）：力矩使顶部向力方向，底部反向
  在质心以下施力（推箱子底部）：容易翻倒
```

---

## 二、典型应用

```cpp
// 1. 门被风吹（均匀分布力 = 多点施力）
void ApplyWindToDoor(UPrimitiveComponent* Door, FVector WindForce)
{
    // 在门的多个点施加风力（模拟分布载荷）
    FVector Center = Door->GetComponentLocation();
    FVector Up     = Door->GetUpVector();
    FVector Right  = Door->GetRightVector();
    
    for (int i = -1; i <= 1; i += 2)
    for (int j = -1; j <= 1; j += 2)
    {
        FVector Point = Center + Right * (i * 40.f) + Up * (j * 80.f);
        Door->AddForceAtLocation(WindForce * 0.25f, Point);
    }
}

// 2. 起重机钢缆拉力（精确点施力）
void ApplyCraneForce(UPrimitiveComponent* Load,
    FVector CableAttachPoint,  // 钢缆在 Load 上的绑点
    FVector TensionForce)       // 绳子张力
{
    Load->AddForceAtLocation(TensionForce, CableAttachPoint);
    // → 如果绑点不在质心，会产生旋转力矩（真实效果）
}

// 3. 喷气推进（引擎在非质心位置）
void ApplyJetThrust(UPrimitiveComponent* Rocket)
{
    // 引擎在火箭尾部
    FVector ThrustPoint = Rocket->GetComponentLocation()
                        - Rocket->GetForwardVector() * 200.f;  // 尾部
    FVector Thrust = Rocket->GetForwardVector() * 500000.f;
    Rocket->AddForceAtLocation(Thrust, ThrustPoint);
    // 推力施在尾部 → 推力矩使机头转向
    // （多引擎火箭需要精确控制各引擎推力以保持直飞）
}
```

---

## 三、延伸阅读

- 📄 [4.7 速度直接控制](./07-velocity-control.md)
- 📄 [4.8 质心高级应用](./08-center-of-mass-advanced.md)
