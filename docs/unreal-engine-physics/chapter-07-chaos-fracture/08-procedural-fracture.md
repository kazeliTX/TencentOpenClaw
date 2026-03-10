# 7.8 程序化破碎运行时

> **难度**：⭐⭐⭐⭐⭐

## 一、运行时触发破碎

```cpp
// 直接在 C++ 中触发 GC 破碎（不需要 Field Actor）
void TriggerFractureAtPoint(
    UGeometryCollectionComponent* GCC,
    FVector WorldPoint,
    float   StrainMagnitude)
{
    // 将世界坐标转为局部坐标
    FTransform InvTransform = GCC->GetComponentTransform().Inverse();
    FVector LocalPoint = InvTransform.TransformPosition(WorldPoint);

    // 获取物理代理（Chaos 后端）
    if (Chaos::FPhysicsSolver* Solver =
        GCC->GetWorld()->GetPhysicsScene()->GetSolver())
    {
        // 通过 ExternalStrain 施加应变
        // UE5.1+ 推荐使用 FieldSystem Actor 而非直接调用 Solver
    }
    
    // 推荐方式：生成临时 FieldSystem Actor
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    AFieldSystemActor* FA = GCC->GetWorld()->SpawnActor<AFieldSystemActor>(
        AFieldSystemActor::StaticClass(),
        WorldPoint, FRotator::ZeroRotator, Params);
    if (FA)
    {
        // 配置 Field 参数（见 04-field-system.md）
        FA->SetLifeSpan(0.05f);  // 下帧即销毁
    }
}
```

## 二、分步破碎（渐进式结构倒塌）

```
设计模式：Cascade Fracture（瀑布式破碎）

  爆炸点触发 → Level 0 碎片分开
             → 碎片撞击其他 GC → 触发更多破碎
             → 连锁反应，像真实建筑倒塌
             
实现要点：
  1. 开启 GCComp 的 "Enable Clustering Collisions"
     → 碎片飞出后可以与其他 GC 碰撞并触发其破碎

  2. Damage Propagation Factor：
     一块碎片破碎时，向相邻连接传播的应变比例
     = 0.0 → 不传播（每块独立破碎）
     = 1.0 → 完全传播（一触即溃）
     推荐：0.3~0.5（有连锁但不夸张）
     
  3. 破碎传播延迟：
     用 Timer 控制连锁破碎的时序
     → 产生"从爆点向外扩散"的视觉效果
```

## 三、延伸阅读

- 📄 [7.9 破碎 LOD](./09-fracture-lod.md)
- 📄 [代码示例：破碎管理器](./code/03_destruction_manager.cpp)
