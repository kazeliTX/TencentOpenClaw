# 4.8 质心高级应用

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 30 分钟

---

## 一、质心动态偏移

```
质心偏移的动态调整场景：

1. 车辆重心转移（载重/燃油消耗）
   空车：质心在中间
   后排乘客坐入：质心向后移动
   → 更容易在加速时前轮离地
   
2. 角色携带重物
   右手持重剑：质心向右偏
   → 物理布娃娃时倒向右侧

3. 火箭燃料消耗
   燃料舱空：质心上移
   → 动态更新惯性张量

动态更新质心（C++）：
  FBodyInstance* BI = Mesh->GetBodyInstance();
  BI->COMNudge = NewOffset;
  BI->UpdateMassProperties();  // 重新计算质心和惯性张量

性能注意：UpdateMassProperties 开销中等
  不要每帧调用
  仅在质量分布发生变化时调用（如拾取/放下重物）
```

---

## 二、复合物体质心

```
多个物理体组合时质心的计算：

TArray<UPrimitiveComponent*> Parts = GetAllPhysicsParts();
FVector TotalCoM = FVector::ZeroVector;
float   TotalMass = 0.f;

for (UPrimitiveComponent* Part : Parts)
{
    if (!Part->IsSimulatingPhysics()) continue;
    float M  = Part->GetMass();
    FVector C = Part->GetBodyInstance()->GetCOMPosition();
    TotalCoM += C * M;
    TotalMass += M;
}
if (TotalMass > 0.f)
    TotalCoM /= TotalMass;
// TotalCoM = 整体质心

// 可视化质心
DrawDebugSphere(GetWorld(), TotalCoM, 10.f, 8, FColor::Red, false, 1.f);
```

---

## 三、质心与旋转稳定性

```
质心位置决定物体的翻倒难易程度：

静态稳定性（Static Stability）：
  支撑多边形（Support Polygon）= 接触地面的支撑点围成的区域
  质心投影在支撑多边形内 → 稳定（不翻倒）
  质心投影在支撑多边形外 → 不稳定（会翻倒）
  
  质心越低（相对于支撑宽度）→ 越稳定
  翻倒角 = arctan(支撑宽度/2 / 质心高度)

游戏应用：
  坦克（低质心/宽车体）：翻倒角 > 60°，极稳定
  摩托车（单轮/高质心）：翻倒角 < 5°，需要陀螺效应维持平衡
  高塔（高质心/窄底）：翻倒角 < 10°，轻推即倒

Chaos Debug 查看质心：
  p.Chaos.DebugDraw.ShowCenterOfMass 1
  → 红色十字标记每个物理体的质心
```

---

## 四、延伸阅读

- 📄 [4.9 物理模拟事件](./09-physical-simulation-events.md)
- 📄 [4.10 Kinematic 驱动](./10-kinematic-driven.md)
