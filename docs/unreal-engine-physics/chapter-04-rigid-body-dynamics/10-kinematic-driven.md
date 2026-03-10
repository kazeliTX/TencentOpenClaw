# 4.10 Kinematic 驱动与混合模式

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 35 分钟

---

## 一、Kinematic vs Simulated

```
Simulated（物理模拟）：
  Chaos 控制位置/旋转
  受力/重力/碰撞影响
  无法直接设置变换（会被物理覆盖）

Kinematic（运动学）：
  游戏代码控制位置/旋转
  不受力/重力影响
  但参与碰撞（会推开其他物理物体！）
  
  SetSimulatePhysics(false) = Kinematic
  SetSimulatePhysics(true)  = Simulated

Kinematic 的特殊能力：
  可以像平台/障碍物一样移动并推开物理物体
  移动电梯：Kinematic + 每帧移动变换 → 物理物体站在上面会被带动
  动画驱动碰撞：AnimBP 驱动骨骼 + Kinematic → 挥舞武器打飞物体

切换时的速度保持：
  Kinematic → Simulated 时，Chaos 可以获取运动速度：
    BI->SetKinematicTarget(Transform, true); // true=设置速度
    当切换为 Simulated 时，物体保持运动速度继续运动（抛出感觉）
```

---

## 二、SetKinematicTarget

```cpp
// Kinematic 物体精确移动
void MoveKinematicObject(UPrimitiveComponent* Comp, FTransform NewTransform)
{
    FBodyInstance* BI = Comp->GetBodyInstance();
    if (!BI) return;
    
    // 不是直接设置 Actor 变换！
    // 而是通过物理接口设置 Kinematic 目标
    // → Chaos 在下一帧将物体移到目标位置，同时更新速度（用于碰撞计算）
    BI->SetBodyTransform(NewTransform, ETeleportType::TeleportPhysics);
    // ETeleportType::TeleportPhysics：设置位置但清零速度
    // ETeleportType::None：设置位置并估算速度（推荐用于移动平台）
}

// 移动平台（Kinematic）
class AMovingPlatform : public AActor
{
    FVector StartPos, EndPos;
    float Speed = 100.f;
    float Alpha = 0.f;

    void Tick(float Dt)
    {
        Alpha = FMath::Fmod(Alpha + Dt * Speed / FVector::Distance(StartPos, EndPos), 1.f);
        float T = FMath::Sin(Alpha * PI);  // 来回运动
        FVector NewLoc = FMath::Lerp(StartPos, EndPos, T);
        
        // 用 SetActorLocation（Sweep=false）+ 物理 Teleport
        SetActorLocation(NewLoc, false, nullptr, ETeleportType::None);
        // ETeleportType::None → Chaos 计算速度 = ΔPos / Δt
        // → 站在平台上的物理对象会被"推着走"
    }
};
```

---

## 三、物理模拟状态混合

```
布娃娃混合（Blend Physics）：
  动画姿势 + 物理模拟的权重混合
  0.0 = 完全动画驱动
  1.0 = 完全物理模拟
  中间值 = 混合（BlendPhysics）
  
  UE 动画系统支持：
    AnimBP → "Physics Blend Weight" 节点
    或 USkeletalMeshComponent::PhysicsBlendWeight
  
  应用：
    角色中弹 → PhysicsBlendWeight 逐渐从 0 增大到 1（缓慢变布娃娃）
    布娃娃站起 → 反向，从 1 缓慢减小到 0（从布娃娃回到动画）

// 逐渐变为布娃娃
void TransitionToRagdoll(float Duration)
{
    GetWorld()->GetTimerManager().SetTimer(RagdollTimer,
        FTimerDelegate::CreateLambda([this, Duration]()
        {
            static float Elapsed = 0.f;
            Elapsed += 0.05f;
            float Alpha = FMath::Clamp(Elapsed / Duration, 0.f, 1.f);
            GetMesh()->PhysicsBlendWeight = Alpha;
            if (Alpha >= 1.f)
            {
                GetMesh()->SetSimulatePhysics(true);
                GetWorld()->GetTimerManager().ClearTimer(RagdollTimer);
            }
        }), 0.05f, true);
}

// 从布娃娃站起（Pose Snapshot）
void GetUpFromRagdoll()
{
    // 1. 保存布娃娃当前姿势
    FPoseSnapshot RagdollPose;
    GetMesh()->SnapshotPose(RagdollPose);
    
    // 2. 关闭物理模拟
    GetMesh()->SetSimulatePhysics(false);
    
    // 3. 播放起身动画（从 RagdollPose 混合到站立动画）
    PlayGetUpAnimation(RagdollPose);
}
```

---

## 四、延伸阅读

- 📄 [代码示例：Kinematic/Physics 混合](./code/03_kinematic_blend.cpp)
- 📄 [第五章：Physics Asset & Ragdoll](../chapter-05-physics-asset-ragdoll/)
