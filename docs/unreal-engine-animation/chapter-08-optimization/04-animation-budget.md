# 8.4 Animation Budget Allocator

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、什么是 Animation Budget Allocator

```
问题：
  场景中有 200 个角色
  即使用了 LOD，同时都在屏幕内还是太慢

Animation Budget Allocator 解决：
  设置每帧动画的总 CPU 预算（如 2ms）
  系统自动决定哪些角色以全速更新，哪些降频
  基于角色的重要性（距离、是否可见、是否重要）动态分配

效果：
  无论场景有多少角色，动画 CPU 开销始终控制在预算内
  重要/近处角色：全速 60fps 更新
  次要/远处角色：自动降为 30fps 或 15fps 更新
  最不重要的角色：每隔 3~4 帧更新一次甚至更低
```

---

## 二、启用 Animation Budget Allocator

```
1. 启用插件：
   Edit → Plugins → Animation → Animation Budget Allocator ✅

2. 在 Character 的 SkeletalMeshComponent 上启用：
   SkeletalMeshComponent → Optimization
   → Component Use Fixed Skel Bounds: ✅（避免逐帧计算 Bounds）
   → Auto Calculate LOD Screen Size: ✅

3. 创建 Animation Budget Allocator 资产：
   内容浏览器 → 右键 → Miscellaneous → Data Asset
   选择 AnimationBudgetAllocatorParameters
   命名：DA_AnimBudget
   
4. 在项目设置中引用：
   Project Settings → Engine → Animation Budget Allocator
   → Budget Allocator Parameters: DA_AnimBudget
```

---

## 三、Budget 参数配置

```
打开 DA_AnimBudget 资产：

Total Budget（总预算）：
  Budget (ms): 2.0       ← 每帧最多允许多少 ms 用于动画
                             （根据目标帧率调整，60fps 建议 2~3ms）
  Max Tick Rate: 30       ← 最大动画更新频率（不超过 30fps）
  Min Tick Rate: 15       ← 最低动画更新频率（即使超预算也不低于此）

优先级参数：
  State Change Smoothing Time: 0.25s
    （角色从高优先级到低优先级的过渡时间，防止抖动）
  
  Significance Scale:
    View Dot Scale: 1.0   （朝向摄像机的角色优先级加成）
    Screen Area Scale: 1.0（屏幕占用大的角色优先级加成）
    Distance Scale: 0.5   （距离对优先级的影响权重）

Debug：
  Debug Draw: ✅（显示每个角色的动画更新频率）
  → 绿色：全速更新  黄色：降频更新  红色：最低频率
```

---

## 四、C++ 中手动设置重要性

```cpp
// 某些角色需要强制高优先级（如 BOSS、玩家队友）
void AMyCharacter::SetAnimationImportance(bool bHighImportance)
{
    USkeletalMeshComponent* Mesh = GetMesh();
    if (!Mesh) return;

    if (bHighImportance)
    {
        // 强制高优先级（不受 Budget 降频影响）
        Mesh->SetComponentTickInterval(0.0f); // 0 = 每帧更新
        // 或者：设置 Significance Manager 分数
    }
    else
    {
        // 允许 Budget Allocator 控制更新频率
        Mesh->SetComponentTickInterval(-1.0f); // -1 = 由系统决定
    }
}

// 注册到 Significance Manager（Budget Allocator 内部使用）
// 高 Significance Score → 更高动画更新优先级
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (USignificanceManager* SigMgr = FSignificanceManagerModule::Get(GetWorld()))
    {
        SigMgr->RegisterObject(
            this,
            FName("Character"),
            [](USignificanceManager::FManagedObjectInfo* Info,
               const FTransform& ViewTransform) -> float
            {
                // 返回重要性分数（越高越优先）
                ACharacter* C = Cast<ACharacter>(Info->GetObject());
                if (!C) return 0.f;
                float Dist = FVector::Dist(C->GetActorLocation(), ViewTransform.GetLocation());
                return FMath::Clamp(1000.f / FMath::Max(Dist, 1.f), 0.f, 1.f);
            });
    }
}
```

---

## 五、延伸阅读

- 📄 [代码示例：Budget 配置](./code/01_anim_budget_setup.cpp)
- 🔗 [Animation Budget Allocator 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-budget-allocator-in-unreal-engine)
