# 2.3 碰撞响应（Block / Overlap / Ignore）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、Collision Enabled 枚举

```
ECollisionEnabled（碰撞启用模式）：

  NoCollision：
    完全禁用碰撞（不参与任何物理或查询）
    性能最优，用于纯视觉 Mesh
  
  QueryOnly：
    只参与查询（LineTrace/Overlap检测）
    不参与物理模拟（不受力/不阻挡物理对象）
    触发区域（ATriggerVolume）使用此模式
  
  PhysicsOnly：
    只参与物理模拟（受力/阻挡其他物理对象）
    不参与 LineTrace 查询
    用于纯物理但不需要射线检测的对象
  
  QueryAndPhysics（默认）：
    同时参与查询和物理
    完整碰撞功能
    性能最低（但最常用）
  
  ProbeOnly（UE5.1+）：
    探针模式，接收重叠但不阻挡任何物体
    轻量级传感器，比 QueryOnly+Overlap 更高效
    适合大量密集传感器（检查点、拾取物）

C++ 设置：
  Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
```

---

## 二、Hit 事件完整数据

```
FHitResult 详细字段：

  bBlockingHit：   是否是阻挡碰撞（true=Block，false=Overlap）
  bStartPenetrating：碰撞开始时是否已经穿入（重叠起始）
  
  距离信息：
    Distance：      从起点到碰撞点的距离（cm）
    Time：          [0,1] 归一化距离（Distance/TraceLength）
    Location：      碰撞发生时，被测体的世界位置
    ImpactPoint：   实际碰撞点（世界坐标）
    ImpactNormal：  碰撞面法线（从被击中表面指向外）
    Normal：        碰撞体移动方向的法线（通常与ImpactNormal相同）
  
  对象信息：
    Actor：         被击中的 AActor
    Component：     被击中的 UPrimitiveComponent
    BoneName：      骨骼网格中被击中的骨骼名称（非骨骼=None）
    FaceIndex：     Complex Collision 中被击中的三角面索引
    ElementIndex：  多材质 Mesh 中的材质索引
    
  物理信息：
    PhysMaterial：  被击中表面的物理材质（弱指针）
    PenetrationDepth：穿透深度（bStartPenetrating=true时有效）

使用示例：
  void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
      UPrimitiveComponent* OtherComp, FVector NormalImpulse,
      const FHitResult& Hit)
  {
      // 播放撞击音效（基于物理材质）
      if (Hit.PhysMaterial.IsValid())
      {
          UPhysicalMaterial* PM = Hit.PhysMaterial.Get();
          // PM->SurfaceType → ESurfaceType 枚举，对应脚步音效
      }
      
      // 产生弹孔贴花
      UGameplayStatics::SpawnDecalAtLocation(
          GetWorld(), BulletHoleDecal,
          FVector(5,5,0.1f),
          Hit.ImpactPoint,
          Hit.ImpactNormal.Rotation());
      
      // 施加击退力
      OtherComp->AddImpulseAtLocation(
          -Hit.ImpactNormal * 50000.f,
          Hit.ImpactPoint);
  }
```

---

## 三、Overlap 事件完整流程

```
Overlap 触发条件：
  1. Component.CollisionEnabled = QueryOnly 或 QueryAndPhysics
  2. Component.bGenerateOverlapEvents = true
  3. 双方通道响应组合结果为 Overlap（见 2.2）

事件对（对称触发）：
  BeginOverlap：两对象开始重叠
    → Actor 上的 OnActorBeginOverlap
    → Component 上的 OnComponentBeginOverlap
  
  EndOverlap：两对象停止重叠
    → Actor 上的 OnActorEndOverlap
    → Component 上的 OnComponentEndOverlap

Overlap 回调参数：
  void OnBeginOverlap(
      UPrimitiveComponent* OverlappedComp,  // 自己的碰撞体
      AActor* OtherActor,                    // 对方 Actor
      UPrimitiveComponent* OtherComp,        // 对方碰撞体
      int32 OtherBodyIndex,                  // 对方骨骼体索引
      bool bFromSweep,                       // 是否来自 Sweep 移动
      const FHitResult& SweepResult)         // 如果 bFromSweep，附带 Hit 信息

注意事项：
  Overlap 事件在游戏线程触发（EndPhysics 阶段）
  同帧内可能触发多次（多个对象同时进入）
  BeginOverlap 必须配对 EndOverlap（防止逻辑错误）
  
  TArray<AActor*> OverlappingActors；
  Comp->GetOverlappingActors(OverlappingActors);  // 查询当前所有重叠 Actor
```

---

## 四、Blocking Hit vs Overlap 的性能差异

```
性能层级（从低到高开销）：

  Ignore:     最优（BroadPhase 直接过滤，不进入 NarrowPhase）
  Overlap:    中等（需要 NarrowPhase 计算接触，但不解算约束）
  Block:      最高（NarrowPhase + 约束求解 + 去穿透 + 事件触发）

优化建议：
  大量密集对象（草地/粒子）→ 优先使用 Ignore
  触发区域（拾取/检查点）→ Overlap 而非 Block
  只需要一次 BeginOverlap → 触发后关闭 bGenerateOverlapEvents
  
  // 关闭 Overlap 生成（性能优化）
  Comp->SetGenerateOverlapEvents(false);
  
  // 触发后只处理一次（拾取物品）
  void OnPickup(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, ...)
  {
      if (bPickedUp) return;
      bPickedUp = true;
      Comp->SetGenerateOverlapEvents(false);  // 立即关闭，避免重复触发
      // 执行拾取逻辑...
  }
```

---

## 五、运行时修改响应

```
C++ 动态修改碰撞响应：

  // 修改单个通道响应
  Comp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
  
  // 修改所有通道（先全设为一个值，再单独调整）
  Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
  Comp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
  
  // 完整碰撞配置（批量设置）
  FCollisionResponseContainer ResponseContainer;
  ResponseContainer.SetAllChannels(ECR_Ignore);
  ResponseContainer.SetResponse(ECC_WorldStatic, ECR_Block);
  ResponseContainer.SetResponse(ECC_Pawn, ECR_Overlap);
  Comp->SetCollisionResponseToChannels(ResponseContainer);
  
  // 设置对象通道类型
  Comp->SetCollisionObjectType(ECC_PhysicsBody);
  
  // 整体切换 Profile（切换预设）
  Comp->SetCollisionProfileName(FName("Projectile"));
```

---

## 六、延伸阅读

- 📄 [2.5 Hit Event 详解](./05-hit-events.md)
- 📄 [2.6 Overlap Event 详解](./06-overlap-events.md)
- 🔗 [Collision Response Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-response-reference-in-unreal-engine)
