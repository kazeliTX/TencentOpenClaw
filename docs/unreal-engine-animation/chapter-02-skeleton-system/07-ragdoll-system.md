# 2.7 布娃娃系统实现

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、布娃娃的三种模式

```
布娃娃模式分类：

1. 完全物理（Full Ragdoll）
   所有骨骼完全由 Chaos Physics 驱动
   使用场景：角色死亡、从高处坠落
   触发：Mesh->SetSimulatePhysics(true)

2. 物理与动画混合（Blended Ragdoll）
   动画占比 X%，物理占比 (1-X)%
   使用场景：受击晃动（Hit Reaction）
   实现：Physical Animation Component

3. 部分物理（Partial Ragdoll）
   只有特定骨骼链使用物理（如：下半身动画，上半身物理）
   使用场景：被压制时腿部继续走路，上身布娃娃
   实现：SetAllBodiesBelowSimulatePhysics
```

---

## 二、完整布娃娃实现

```cpp
// RagdollComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RagdollComponent.generated.h"

UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API URagdollComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URagdollComponent();

    /** 启用完全布娃娃 */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnableRagdoll(FVector DeathImpulse = FVector::ZeroVector);

    /** 从布娃娃恢复到动画（站起来） */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DisableRagdoll(float BlendOutTime = 0.5f);

    /** 部分布娃娃（指定骨骼以下） */
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnablePartialRagdoll(FName BoneName);

    /** 检查当前是否处于布娃娃状态 */
    UFUNCTION(BlueprintPure, Category = "Ragdoll")
    bool IsRagdollActive() const { return bRagdollActive; }

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> MeshComp;

    bool bRagdollActive = false;
    FTimerHandle BlendOutTimerHandle;

    // 保存布娃娃前的动画 Transform（用于平滑过渡）
    FTransform SavedMeshRelativeTransform;

    void OnBlendOutTimerFired();
};

// ─────────────────────────────────────────────────────────────
// RagdollComponent.cpp
// ─────────────────────────────────────────────────────────────

void URagdollComponent::BeginPlay()
{
    Super::BeginPlay();
    MeshComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
}

void URagdollComponent::EnableRagdoll(FVector DeathImpulse)
{
    if (!MeshComp || bRagdollActive) return;

    bRagdollActive = true;

    // 1. 保存当前 Mesh 的相对变换（恢复时用）
    SavedMeshRelativeTransform = MeshComp->GetRelativeTransform();

    // 2. 开启物理模拟
    MeshComp->SetAllBodiesSimulatePhysics(true);
    MeshComp->SetSimulatePhysics(true);
    MeshComp->WakeAllRigidBodies();

    // 3. 碰撞设置
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionProfileName(FName("Ragdoll"));  // 需要预先配置此 Profile

    // 4. 从 Capsule 组件 Detach（让 Mesh 自由飞）
    MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

    // 5. 施加死亡冲量（可选）
    if (!DeathImpulse.IsNearlyZero())
    {
        MeshComp->AddImpulse(DeathImpulse, NAME_None, true);
    }

    UE_LOG(LogTemp, Log, TEXT("Ragdoll enabled for %s"), *GetOwner()->GetName());
}

void URagdollComponent::DisableRagdoll(float BlendOutTime)
{
    if (!MeshComp || !bRagdollActive) return;

    // 1. 找到最近的地面骨骼位置（用于 Mesh 重新定位）
    FVector HipLocation = MeshComp->GetBoneLocation(FName("pelvis"));

    // 2. 获取 Owner（通常是 ACharacter）并移动到骨盆位置
    ACharacter* Owner = Cast<ACharacter>(GetOwner());
    if (Owner)
    {
        FVector NewActorLocation = HipLocation;
        NewActorLocation.Z -= Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        Owner->SetActorLocation(NewActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    // 3. 重新 Attach Mesh
    MeshComp->AttachToComponent(
        Owner ? Owner->GetRootComponent() : GetOwner()->GetRootComponent(),
        FAttachmentTransformRules::KeepWorldTransform
    );

    // 4. 关闭物理，恢复动画
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetAllBodiesSimulatePhysics(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 5. 恢复原来的相对变换（带平滑过渡）
    // 这里简化处理，直接重置。实际项目需要 Blend 过渡。
    MeshComp->SetRelativeTransform(SavedMeshRelativeTransform);

    bRagdollActive = false;
}

void URagdollComponent::EnablePartialRagdoll(FName BoneName)
{
    if (!MeshComp) return;

    // 指定骨骼及以下所有 Body 开启物理
    MeshComp->SetAllBodiesBelowSimulatePhysics(BoneName, true, true);
    // 参数：骨骼名，是否包含自身，是否递归子骨骼
}
```

---

## 三、物理动画混合（Hit Reaction）

```cpp
// 使用 Physical Animation Component 实现受击物理混合
// 这是 Epic 官方推荐的轻量混合方案

// 在角色类中：
UPROPERTY()
UPhysicalAnimationComponent* PhysicalAnim;

void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    PhysicalAnim = FindComponentByClass<UPhysicalAnimationComponent>();
    if (PhysicalAnim)
    {
        // 配置物理动画参数
        FPhysicalAnimationData AnimData;
        AnimData.bIsLocalSimulation       = false;
        AnimData.OrientationStrength      = 1000.0f;  // 跟随动画旋转的弹力
        AnimData.AngularVelocityStrength  = 100.0f;   // 角速度阻尼
        AnimData.PositionStrength         = 1000.0f;  // 跟随动画位置的弹力
        AnimData.VelocityStrength         = 100.0f;   // 速度阻尼
        AnimData.MaxLinearForce           = 0.0f;     // 0 = 无限制
        AnimData.MaxAngularForce          = 0.0f;

        // 对 spine_03 以上的骨骼（上半身）应用物理动画
        PhysicalAnim->SetSkeletalMeshComponent(GetMesh());
        PhysicalAnim->ApplyPhysicalAnimationProfileBelow(
            FName("spine_03"),
            NAME_None,  // 使用默认 Profile
            true,       // 包含自身
            false       // 不包含动画姿势驱动（改用代码驱动）
        );

        // 开启物理模拟（仅上半身）
        GetMesh()->SetAllBodiesBelowSimulatePhysics(
            FName("spine_03"), true, true);
    }
}

// 受击时触发
void AMyCharacter::OnHit(FVector HitImpulse, FName HitBone)
{
    // 施加冲量到被击中的骨骼
    GetMesh()->AddImpulse(HitImpulse, HitBone, true);

    // 延迟恢复（1秒后上半身物理恢复动画控制）
    GetWorldTimerManager().SetTimer(
        RecoverTimerHandle,
        this,
        &AMyCharacter::OnHitRecovered,
        1.0f,
        false
    );
}

void AMyCharacter::OnHitRecovered()
{
    GetMesh()->SetAllBodiesBelowSimulatePhysics(
        FName("spine_03"), false, true);
}
```

---

## 四、布娃娃常见问题

| 问题 | 原因 | 解决方案 |
|------|------|--------|
| 角色瞬间穿地 | 碰撞体配置太小/位置偏 | 调整 Physics Body 大小，确保覆盖 Mesh |
| 关节无限旋转 | 约束角度限制太宽/缺少阻尼 | 收紧 Swing/Twist 限制，增加 Angular Damping |
| 布娃娃弹飞 | 冲量值过大 | 测试合适的 Impulse 强度，通常 10000-50000 |
| 从布娃娃站起来抖动 | 骨骼位置与胶囊体不对齐 | 在恢复前先重置 Mesh 位置到胶囊体中心 |
| 多角色布娃娃卡顿 | 物理 Body 数量过多 | LOD 裁减 Body，死亡后延迟销毁 |

---

## 五、延伸阅读

- 📄 [2.6 Physics Asset 配置](./06-physics-asset.md)
- 🔗 [Physical Animation Component 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/physical-animation-in-unreal-engine)
- 🔗 [Ragdoll 官方示例](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-asset-editor-in-unreal-engine)
