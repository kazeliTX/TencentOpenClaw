# 2.6 Overlap Event 详解

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、Overlap 工作原理

```
Overlap 判断流程：

  每帧 EndPhysics 阶段：
    1. 收集所有 bGenerateOverlapEvents=true 的 Component
    2. 用 BroadPhase 快速筛选可能重叠的对象对
    3. 对候选对进行 NarrowPhase 精确检测
    4. 对比"当前帧重叠集"与"上帧重叠集"：
       新增对象 → 触发 BeginOverlap
       消失对象 → 触发 EndOverlap
    5. 更新"上帧重叠集"为当前帧结果

重叠追踪数据结构（内部）：
  TArray<FOverlapInfo> CurrentOverlaps;   // 当前帧重叠
  TArray<FOverlapInfo> PreviousOverlaps;  // 上帧重叠
  // 差集 = 需要触发 Begin/End 的对

性能注意：
  bGenerateOverlapEvents 的 Component 越多，开销越大
  静止的 QueryOnly Component 如果永不移动 → 可以关闭 Tick
  特别大的场景（草地/花朵）→ 禁用 GenerateOverlapEvents
```

---

## 二、Begin/End Overlap 完整示例

```cpp
UCLASS()
class ATriggerZone : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) USphereComponent* TriggerSphere;
    TSet<AActor*> ActorsInZone;

    ATriggerZone()
    {
        TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
        TriggerSphere->SetSphereRadius(200.f);
        TriggerSphere->SetCollisionProfileName(FName("OverlapAllDynamic"));
        TriggerSphere->SetGenerateOverlapEvents(true);
        RootComponent = TriggerSphere;
    }

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        TriggerSphere->OnComponentBeginOverlap.AddDynamic(
            this, &ATriggerZone::OnOverlapBegin);
        TriggerSphere->OnComponentEndOverlap.AddDynamic(
            this, &ATriggerZone::OnOverlapEnd);
    }

    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult)
    {
        if (!OtherActor || OtherActor == this) return;
        ActorsInZone.Add(OtherActor);
        UE_LOG(LogTemp, Log, TEXT("%s entered zone. Total: %d"),
            *OtherActor->GetName(), ActorsInZone.Num());
    }

    UFUNCTION()
    void OnOverlapEnd(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex)
    {
        if (!OtherActor) return;
        ActorsInZone.Remove(OtherActor);
        UE_LOG(LogTemp, Log, TEXT("%s left zone. Total: %d"),
            *OtherActor->GetName(), ActorsInZone.Num());
    }

    // 查询当前所有重叠对象（不依赖事件）
    TArray<AActor*> GetCurrentOverlaps() const
    {
        TArray<AActor*> Result;
        TriggerSphere->GetOverlappingActors(Result);
        return Result;
    }
};
```

---

## 三、Overlap 的边界情况处理

```
问题 1：Actor 生成时已在触发器内
  情况：角色 BeginPlay 时就在触发器范围内
  行为：不触发 BeginOverlap！（需要主动查询）
  
  解决：在 BeginPlay 中主动检查：
    TArray<AActor*> Overlapping;
    TriggerSphere->GetOverlappingActors(Overlapping, ACharacter::StaticClass());
    for (AActor* A : Overlapping)
        HandleActorEnter(A);

问题 2：Actor 传送（瞬移）穿过触发器
  情况：SetActorLocation 瞬间移动，不经过触发器
  行为：可能不触发 Overlap
  
  解决：用 Sweep=true 的 SetActorLocation：
    SetActorLocation(NewLocation, true); // true = bSweep

问题 3：EndOverlap 未触发（Actor 被 Destroy）
  情况：Actor 在重叠状态被销毁
  行为：EndOverlap 可能不触发
  
  解决：在 OnOverlapEnd 中过滤无效 Actor：
    if (!IsValid(OtherActor)) return;
    在触发器的 Tick 中周期性清理失效 Actor：
    ActorsInZone.RemoveAll([](AActor* A){ return !IsValid(A); });

问题 4：多 Component 的 Actor 重叠
  情况：有多个碰撞体的 Actor 进入触发器
  行为：每个 Component 都会触发 BeginOverlap（同一个 Actor 触发多次！）
  
  解决：只处理第一次（检查 ActorsInZone 是否已有该 Actor）：
    if (ActorsInZone.Contains(OtherActor)) return;
```

---

## 四、延伸阅读

- 📄 [2.7 Trace Channel 详解](./07-trace-channels.md)
- 📄 [代码示例：Hit & Overlap](./code/02_hit_overlap_events.cpp)
