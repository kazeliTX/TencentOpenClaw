# 3.10 AnimBP 性能优化实践

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、AnimBP 性能分析

```bash
# 常用分析命令
stat anim           # 动画系统整体耗时
stat animverbose    # 详细信息（每个 AnimBP）
stat game           # Game Thread 整体（含 NativeUpdateAnimation）

# Unreal Insights
# 启动时加参数：-trace=cpu,anim
# 关注：AnimTask（Worker Thread），GameThread（Event Graph）
```

---

## 二、Top 10 性能陷阱

### 陷阱 1：每帧 Cast（最常见）

```cpp
// ❌ 极坏
void UBadAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    AMyCharacter* Char = Cast<AMyCharacter>(TryGetPawnOwner()); // 每帧 Cast！
    if (Char) GroundSpeed = Char->GetVelocity().Size2D();
}

// ✅ 正确
void UGoodAnimInstance::NativeInitializeAnimation()
{
    CachedChar = Cast<AMyCharacter>(TryGetPawnOwner()); // 只做一次
}
void UGoodAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    if (!CachedChar) return;
    GroundSpeed = CachedChar->GetVelocity().Size2D();
}
```

### 陷阱 2：忘记 LOD Threshold

```
问题：远处 LOD2/3 角色仍在执行完整 IK 计算
解决：
  AnimGraph 中所有 IK 节点：
  Details → LOD Threshold = 1
  （LOD > 1 时跳过，不执行）
  
  状态机中的特殊节点（如 Look At）：
  Details → LOD Threshold = 0
  （LOD > 0 时跳过）
```

### 综合优化清单

```
□ 所有引用在 NativeInitializeAnimation 中缓存
□ NativeUpdateAnimation 添加早退条件（!CachedChar return）
□ 将复杂数学计算移到 NativeThreadSafeUpdateAnimation
□ 所有 IK 节点设置 LOD Threshold
□ 状态机中减少不必要的 Blend（合并相似条件）
□ 开启 OnlyTickPoseWhenRendered（可见性驱动更新）
□ 开启 URO（Update Rate Optimization）
□ 使用 Inertialization 替代传统 Blend（减少多状态共存时间）
□ Linked AnimGraph 分离关注点，便于按需禁用
□ 面向多 NPC 场景使用 AnimBudgetAllocator
```

---

## 三、AnimBudgetAllocator（大规模 NPC 优化）

```
AnimBudgetAllocator（动画预算分配器）：
  自动管理场景中所有动画实例的更新频率
  根据屏幕重要性分配"预算"：重要的角色更新更频繁

使用方法：
  1. 在 World Settings 或 Game Instance 中添加：
     UAnimationBudgetAllocatorComponent

  2. 给需要管理的 SkeletalMeshComponent 设置：
     Mesh->bEnableUpdateRateOptimizations = true;
     Mesh->AnimationBudgetHandle = ...; // 由系统分配

  3. 配置预算（每帧可用的动画更新时间）：
     BudgetAllocator->SetAnimationBudget(3.0f); // 3ms/帧

  效果：
    - 近处角色：每帧更新
    - 中距离：每 2-3 帧更新
    - 远处：每 5-8 帧更新，甚至冻结
    - 自动根据帧时间动态调整
```

---

## 四、Worker Thread 最大化利用

```cpp
// NativeThreadSafeUpdateAnimation：让 Worker Thread 做更多工作

// ✅ 可以在此做的工作：
void UMyAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    // 1. 衍生量计算（不访问 UObject）
    SpeedBlendAlpha = FMath::Clamp(
        (GroundSpeed - 300.0f) / 300.0f, 0.0f, 1.0f);

    // 2. 平滑插值
    SmoothedAimPitch = FMath::FInterpTo(
        SmoothedAimPitch, AimPitch, DeltaSeconds, 12.0f);

    // 3. 曲线采样（只访问 UCurveFloat，线程安全）
    if (LeanCurve)
        LeanAmount = LeanCurve->GetFloatValue(GroundSpeed / 600.0f) * 5.0f;
}

// ❌ 不能在此做：
void UMyAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
    // ❌ 访问 UObject
    ACharacter* Char = CachedCharacter.Get();  // 非线程安全！

    // ❌ 访问 World
    GetWorld()->LineTraceSingle(...);            // 非线程安全！

    // ❌ 创建 UObject
    NewObject<USomeObject>();                   // 非线程安全！
}
```

---

## 五、性能参考数据

```
每项优化的预期收益（100 NPC 场景）：

优化措施                           CPU 节省  备注
──────────────────────────────────────────────────────────
缓存引用（避免每帧 Cast）           10-20%   最低成本
OnlyTickPoseWhenRendered           20-40%   一行代码，收益最高
URO 配置                           30-50%   中远距离 NPC
LOD Threshold 裁减 IK              10-15%   仅影响 IK 较重场景
NativeThreadSafeUpdateAnimation    5-15%    解放 Game Thread
AnimBudgetAllocator（大规模）       20-40%   100+ NPC 专用
Inertialization（减少混合时间）     5-10%    降低同时激活状态数
Linked AnimGraph（按需禁用模块）    0-20%    视具体拆分粒度
```

---

## 六、延伸阅读

- 🔗 [Animation Optimization 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine)
- 🔗 [AnimBudgetAllocator 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-budget-allocator-in-unreal-engine)
- 🔗 [UE5 动画系统性能深度分析（知乎）](https://zhuanlan.zhihu.com/p/1927668362969880273)
- 🔗 [Lyra 动画系统分析](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-in-lyra-sample-game-in-unreal-engine)
