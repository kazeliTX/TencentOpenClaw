# 2.7 Trace Channel 与物理查询通道

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、Trace Channel vs Object Channel

```
Trace Channel（追踪通道）：
  描述"这次查询的类型/来源"
  只在 LineTrace/SweepTrace 中使用
  对象设置对 Trace Channel 的响应 → 决定是否被检测到

Object Channel（对象通道）：
  描述"这个对象的物理类型"
  用于 Simulate Physics、Overlap、Object Type Query

区别示例：
  LineTraceSingleByChannel(ECC_Visibility)：
    → 查询所有对 ECC_Visibility 响应为 Block 的对象
    → 通常：墙壁/地面/角色 = Block，触发器/传感器 = Ignore
  
  LineTraceSingleByObjectType(ObjectTypes=[ECC_Pawn])：
    → 只查询 Object Type = ECC_Pawn 的对象
    → 不管对方对哪个 Trace Channel 的响应是什么

选择原则：
  "找到第一个阻挡视线的东西" → ByChannel(ECC_Visibility)
  "找到射线路径上的所有角色" → ByObjectType([ECC_Pawn])
  "武器检测命中目标"         → 自定义 Trace Channel (WeaponTrace)
```

---

## 二、Trace 查询函数完整列表

```
LineTrace（射线）：
  LineTraceSingleByChannel(Start, End, Channel, Params)
  LineTraceSingleByObjectType(Start, End, ObjectTypes, Params)
  LineTraceSingleByProfile(Start, End, ProfileName, Params)
  LineTraceMultiByChannel(...)    ← 返回所有 HitResult
  LineTraceMultiByObjectType(...)
  LineTraceMultiByProfile(...)

SweepTrace（扫掠/形状追踪）：
  SweepSingleByChannel(Start, End, Rot, Channel, CollisionShape, Params)
  SweepSingleByObjectType(...)
  SweepSingleByProfile(...)
  SweepMultiByChannel(...)
  SweepMultiByObjectType(...)
  SweepMultiByProfile(...)
  
  CollisionShape 类型：
    FCollisionShape::MakeSphere(Radius)
    FCollisionShape::MakeCapsule(Radius, HalfHeight)
    FCollisionShape::MakeBox(HalfExtent)

Overlap（重叠查询，只检测是否重叠，不需要路径）：
  OverlapMultiByChannel(Pos, Rot, Channel, Shape, Params)
  OverlapMultiByObjectType(...)
  OverlapMultiByProfile(...)
  OverlapBlockingTestByChannel(...)     ← 只返回 bool（最快）
  OverlapAnyTestByChannel(...)          ← 任意 Overlap 就返回 true

异步版本（不阻塞游戏线程）：
  AsyncLineTraceByChannel(...)          ← 返回 FTraceHandle
  AsyncSweepByChannel(...)
  AsyncOverlapByChannel(...)
  → 用 FTraceHandle 在下帧获取结果
```

---

## 三、FCollisionQueryParams 详解

```cpp
FCollisionQueryParams Params;

// 添加忽略列表
Params.AddIgnoredActor(this);             // 忽略发起者自身
Params.AddIgnoredComponent(MyComp);       // 忽略特定 Component
Params.AddIgnoredActors(TArray<AActor*>); // 批量忽略

// 精度控制
Params.bTraceComplex = false;    // 用 Simple Collision（快）
                                  // true = 用 Complex（精确但慢）
// 返回信息控制
Params.bReturnPhysicalMaterial = true;  // 返回 PhysicsMaterial
Params.bReturnFaceIndex = true;          // 返回三角面索引（需 bTraceComplex=true）
Params.bFindInitialOverlaps = true;      // 包含起点已重叠的对象

// 调试
Params.TraceTag = FName("WeaponTrace"); // 调试显示时标记名称
Params.bDebugQuery = true;              // 输出调试日志（开发时用）

// 自定义过滤器（高级）
Params.AddIgnoredActors({ActorA, ActorB});
// 或用 UCollisionIgnoreComponent
```

---

## 四、自定义武器 Trace Channel 完整示例

```cpp
// DefaultEngine.ini 中配置：
// +DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,
//   DefaultResponse=ECR_Ignore,bTraceType=True,
//   bStaticObject=False,Name="WeaponTrace")

// 武器组件
UCLASS()
class UWeaponComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // 武器伤害射线检测
    TArray<FHitResult> PerformMeleeTrace(
        FVector Start, FVector End, float SweepRadius = 20.f)
    {
        TArray<FHitResult> Hits;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(GetOwner());
        Params.bTraceComplex = false;
        Params.bReturnPhysicalMaterial = true;
        
        // 用自定义 WeaponTrace 通道
        GetWorld()->SweepMultiByChannel(
            Hits, Start, End, FQuat::Identity,
            ECC_GameTraceChannel1,          // WeaponTrace
            FCollisionShape::MakeSphere(SweepRadius),
            Params);
        
        // 过滤重复 Actor（同一个 Actor 只造成一次伤害）
        TSet<AActor*> HitActors;
        TArray<FHitResult> FilteredHits;
        for (const FHitResult& H : Hits)
        {
            if (AActor* A = H.GetActor())
            {
                if (!HitActors.Contains(A))
                {
                    HitActors.Add(A);
                    FilteredHits.Add(H);
                }
            }
        }
        return FilteredHits;
    }
};
```

---

## 五、延伸阅读

- 📄 [2.8 复杂碰撞与优化](./08-complex-collision.md)
- 📄 [代码示例：自定义 Trace Channel](./code/03_custom_trace_channel.cpp)
- 🔗 [Traces in Unreal Engine 5](https://dev.epicgames.com/documentation/en-us/unreal-engine/traces-in-unreal-engine-5)
