# 9.4 物理绳索链条（可交互）

> **难度**：⭐⭐⭐⭐⭐

## 一、物理约束链条原理

```
可交互绳索 = 一串 StaticMesh/Physics Body + PhysicsConstraint
  每节绳段是一个带物理的 Body
  相邻绳段通过 PhysicsConstraint 连接（仅允许弯曲，不允许拉伸）
  
  优点：
    真实碰撞（玩家可以抓住/爬上）
    可以施加力（弓射穿绳子）
    可以断裂（约束超过限制自动断开）
    
  缺点：
    比 UCableComponent 重约 10~50x
    每节段都是独立物理 Body
    16 节绳索 ≈ 16 个刚体 + 15 个约束
```

## 二、运行时生成物理绳索（C++）

```cpp
// 在两点之间生成 N 段物理绳索
void SpawnPhysicsRope(FVector Start, FVector End, int32 Segments)
{
    FVector Delta   = End - Start;
    FVector SegDir  = Delta / Segments;
    float   SegLen  = SegDir.Size();
    
    TArray<AActor*> Nodes;
    
    for (int32 i = 0; i <= Segments; i++)
    {
        FVector Loc = Start + SegDir * i;
        
        // 生成绳段（预制 BlueprintActor 或 StaticMesh）
        AActor* Node = GetWorld()->SpawnActor<APhysicsRopeNode>(
            RopeNodeClass, Loc, FRotator::ZeroRotator);
        Nodes.Add(Node);
        
        if (i > 0)
        {
            // 在相邻节点之间添加 PhysicsConstraint
            APhysicsConstraintActor* Constraint =
                GetWorld()->SpawnActor<APhysicsConstraintActor>();
            
            UPhysicsConstraintComponent* PC =
                Constraint->GetConstraintComp();
            PC->ConstraintActor1 = Nodes[i-1];
            PC->ConstraintActor2 = Nodes[i];
            
            // 允许摆动，禁止扭转和线性移动
            PC->SetAngularSwing1Limit(ACM_Limited, 20.f);
            PC->SetAngularSwing2Limit(ACM_Limited, 20.f);
            PC->SetAngularTwistLimit(ACM_Locked, 0.f);
            PC->SetLinearXLimit(LCM_Locked, 0.f);
            PC->SetLinearYLimit(LCM_Locked, 0.f);
            PC->SetLinearZLimit(LCM_Locked, 0.f);
        }
    }
    
    // 固定顶端（Kinematic）
    if (Nodes.Num() > 0)
    {
        UPrimitiveComponent* TopComp =
            Cast<UPrimitiveComponent>(
                Nodes[0]->GetComponentByClass(UPrimitiveComponent::StaticClass()));
        if (TopComp) TopComp->SetSimulatePhysics(false);
    }
}
```

## 三、延伸阅读

- 📄 [9.5 绳索附加与断裂](./05-rope-attachment.md)
- 📄 [代码示例：软体与绳索](./code/01_physics_rope.cpp)
