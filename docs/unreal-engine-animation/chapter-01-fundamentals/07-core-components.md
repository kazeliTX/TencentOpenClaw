# 1.7 核心组件详解

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 20 分钟

---

## USkeletalMeshComponent

这是整个动画系统的运行时核心，继承链：

```
UObject → UActorComponent → USceneComponent
→ UPrimitiveComponent → UMeshComponent
→ USkinnedMeshComponent → USkeletalMeshComponent
```

### 关键属性与配置

```cpp
// 获取并配置组件
USkeletalMeshComponent* Mesh = GetMesh();

// ── 动画配置 ──
Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint); // 使用 AnimBP
Mesh->SetAnimInstanceClass(MyAnimBPClass);                   // 设置 AnimBP 类

// ── 性能优化 ──
// 重要：只在可见时更新动画（显著降低后台角色开销）
Mesh->VisibilityBasedAnimTickOption =
    EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

// 启用 URO（Update Rate Optimization），远处角色降帧更新
Mesh->EnableUpdateRateOptimizations();
Mesh->SetUpdateAnimationInEditor(true);  // 在编辑器预览时更新

// ── 物理 ──
Mesh->SetSimulatePhysics(false);         // 关闭物理（正常动画模式）
Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

// ── LOD ──
Mesh->SetForcedLOD(0);                  // 0 = 自动，1+ = 强制
```

### 获取 AnimInstance

```cpp
// 获取基础 AnimInstance
UAnimInstance* AnimInst = Mesh->GetAnimInstance();

// 获取并 Cast 到自定义类型（推荐缓存，不要每帧 Cast）
UMyAnimInstance* MyAnimInst = Cast<UMyAnimInstance>(Mesh->GetAnimInstance());

// 获取 Post Process AnimInstance（后处理动画实例）
UAnimInstance* PostAnimInst = Mesh->GetPostProcessInstance();
```

---

## UAnimInstance

动画实例是每个角色专属的动画控制器，运行时由 AnimBP 或 C++ 类实例化。

### 重要函数

```cpp
UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();

// ── Montage 控制 ──
AnimInst->Montage_Play(Montage, 1.0f);
AnimInst->Montage_Stop(0.25f, Montage);
AnimInst->Montage_Pause(Montage);
AnimInst->Montage_Resume(Montage);
AnimInst->Montage_JumpToSection(FName("Loop"), Montage);
bool bPlaying = AnimInst->Montage_IsPlaying(Montage);
FName Section = AnimInst->Montage_GetCurrentSection(Montage);
float Position = AnimInst->Montage_GetPosition(Montage);

// ── 曲线查询 ──
float CurveVal = AnimInst->GetCurveValue(FName("MyAnimCurve"));

// ── 状态机查询 ──
float StateMachineWeight = AnimInst->GetStateMachineInstanceWeight(
    FName("LocomotionSM"));

// ── 骨骼姿势（只读）──
FTransform BoneComp = AnimInst->GetSkelMeshComponent()
    ->GetBoneTransform(BoneIdx);
```

---

## 组件通信模式

```
角色 Gameplay 逻辑
    │
    │（写入状态变量）
    ▼
AnimInstance（NativeUpdateAnimation）
    │
    │（读取状态，驱动 AnimGraph）
    ▼
AnimGraph（Worker Thread 评估）
    │
    │（输出骨骼 Pose）
    ▼
SkeletalMeshComponent（渲染）

原则：
• Gameplay → AnimInstance：单向写入，AnimInstance 是只读消费者
• 不要在 AnimGraph/AnimNode 中调用 Gameplay 函数
• 不要在 Gameplay 中直接修改骨骼变换（应通过 AnimInstance 变量驱动）
```

---

## 延伸阅读

- 🔗 [UAnimInstance API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Animation/UAnimInstance)
- 🔗 [USkeletalMeshComponent API](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/Engine/Components/USkeletalMeshComponent)
- 📄 [代码示例：基础 AnimInstance](../code/01_basic_anim_instance.cpp)
