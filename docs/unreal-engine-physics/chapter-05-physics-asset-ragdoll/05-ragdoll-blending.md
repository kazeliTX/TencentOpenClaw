# 5.5 布娃娃动画混合（BlendPhysics）

> **难度**：⭐⭐⭐⭐⭐

## 一、PhysicsBlendWeight 原理

```
USkeletalMeshComponent::PhysicsBlendWeight：
  [0.0] = 完全动画驱动（AnimBP 控制）
  [1.0] = 完全物理模拟（Chaos 控制）
  [0.5] = 动画姿势 50% + 物理姿势 50%（骨骼空间线性混合）

内部实现：
  每根骨骼：FinalPose = Lerp(AnimPose, PhysPose, BlendWeight)
  BlendWeight 可以是全局（PhysicsBlendWeight），也可以是逐骨骼

AnimBP 中驱动：
  "Set All Bodies Simulate Physics" 节点（在 AnimGraph 中）
  "Blend Physics Bones" 节点：逐骨骼控制混合权重
```

## 二、逐渐变布娃娃（渐变混合）

```cpp
// 平滑过渡到布娃娃
void BeginRagdollBlend(float Duration)
{
    GetMesh()->SetAllBodiesSimulatePhysics(true);
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->PhysicsBlendWeight = 0.f;  // 从动画开始
    
    // 定时逐步增加权重
    float StepTime = 0.033f;  // ~30Hz
    int32 Steps = FMath::CeilToInt(Duration / StepTime);
    float StepAlpha = 1.f / Steps;
    
    int32* Counter = new int32(0);
    GetWorld()->GetTimerManager().SetTimer(BlendTimer,
        FTimerDelegate::CreateLambda([this, Steps, StepAlpha, Counter]()
        {
            (*Counter)++;
            float NewWeight = FMath::Min((*Counter) * StepAlpha, 1.f);
            GetMesh()->PhysicsBlendWeight = NewWeight;
            if (*Counter >= Steps)
            {
                delete Counter;
                GetWorld()->GetTimerManager().ClearTimer(BlendTimer);
            }
        }), StepTime, true);
}
```

## 三、逐骨骼物理混合（Blend Physics Bones）

```
AnimBP AnimGraph 中：
  [Layered Blend Per Bone] → [Blend Physics Bones] → [Output Pose]
  
  Blend Physics Bones 节点：
    Physics Weight（float）：全局权重
    Blend Type：
      SynchedDynamic：物理姿势同步到动画骨骼
      BlendDynamic  ：动画+物理线性混合
    
典型分层：
  受击反应（上半身布娃娃，下半身继续走路动画）：
    Branch 0（权重=0）：根骨骼到 spine_01  —— 腿部动画
    Branch 1（权重=1）：spine_01 以上所有骨骼 —— 上半身物理

AnimBP 蓝图连线：
  EventGraph → 收到命中 → 设置变量 HitBlendAlpha = 1.0
  Update AnimGraph → HitBlendAlpha 驱动 Blend Physics Bones 的 Weight
  过几帧后 HitBlendAlpha 缓慢回到 0（受击效果消失）
```

## 四、延伸阅读

- 📄 [5.6 起身动画](./06-get-up-animation.md)
- 📄 [5.7 受击反应](./07-hit-reaction.md)
