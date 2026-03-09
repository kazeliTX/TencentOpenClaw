// =============================================================================
// 01_bone_modifier.cpp
// 运行时骨骼变换控制器
// 演示：Spring Controller 次级运动、Look At、Bone Driven Controller
// 对应文档：chapter-02-skeleton-system/08-bone-modification.md
// =============================================================================

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BoneModifierComponent.generated.h"

/**
 * 程序化骨骼控制示例组件
 * 展示通过 AnimInstance 变量驱动 AnimGraph 骨骼控制节点的完整模式
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UBoneModifierComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBoneModifierComponent()
    {
        PrimaryComponentTick.bCanEverTick = true;
    }

    // ─────────────────────────────────────────────────────────
    // 脊椎倾斜控制（让角色身体随相机俯仰而倾斜）
    // ─────────────────────────────────────────────────────────

    /** 脊椎额外旋转（用于传递给 AnimBP 的 Transform Modify Bone 节点）*/
    UPROPERTY(BlueprintReadOnly, Category = "BoneControl")
    FRotator SpineTiltRotation = FRotator::ZeroRotator;

    /** 脊椎倾斜强度（1.0 = 100% 跟随相机俯仰）*/
    UPROPERTY(EditAnywhere, Category = "BoneControl", meta = (ClampMin = "0", ClampMax = "1"))
    float SpineTiltStrength = 0.3f;

    /** 脊椎倾斜平滑速度 */
    UPROPERTY(EditAnywhere, Category = "BoneControl")
    float SpineTiltInterpSpeed = 8.0f;

    /** 脊椎俯仰限制（度）*/
    UPROPERTY(EditAnywhere, Category = "BoneControl")
    FVector2D SpinePitchClamp = FVector2D(-15.0f, 25.0f);

    // ─────────────────────────────────────────────────────────
    // 头部注视控制
    // ─────────────────────────────────────────────────────────

    /** 头部 Look At 目标（Component Space，传递给 AnimBP Look At 节点）*/
    UPROPERTY(BlueprintReadOnly, Category = "BoneControl")
    FVector HeadLookAtTargetCS = FVector::ForwardVector * 200.0f;

    /** 是否启用头部注视 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneControl")
    bool bEnableHeadLookAt = true;

    /** 注视目标 Actor（为 nullptr 时注视相机前方）*/
    UPROPERTY(BlueprintReadWrite, Category = "BoneControl")
    TWeakObjectPtr<AActor> LookAtTarget;

    /** 最大转头角度（度）*/
    UPROPERTY(EditAnywhere, Category = "BoneControl")
    float MaxHeadLookAngle = 70.0f;

    /** 注视平滑速度 */
    UPROPERTY(EditAnywhere, Category = "BoneControl")
    float HeadLookInterpSpeed = 5.0f;

    // ─────────────────────────────────────────────────────────
    // 弹簧次级运动（如：后背装备随运动晃动）
    // ─────────────────────────────────────────────────────────

    /** 弹簧骨骼当前偏移（传递给 AnimBP Spring Controller 节点）*/
    UPROPERTY(BlueprintReadOnly, Category = "BoneControl")
    FVector BackpackSpringOffset = FVector::ZeroVector;

    /** 弹簧刚度 */
    UPROPERTY(EditAnywhere, Category = "BoneControl")
    float SpringStiffness = 800.0f;

    /** 弹簧阻尼 */
    UPROPERTY(EditAnywhere, Category = "BoneControl")
    float SpringDamping = 50.0f;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    // 缓存引用
    UPROPERTY()
    TObjectPtr<ACharacter> OwnerCharacter;
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> MeshComp;
    UPROPERTY()
    TObjectPtr<class UMyAnimInstance> AnimInst;  // 替换为你的 AnimInstance 类

    // 弹簧状态
    FVector SpringVelocity = FVector::ZeroVector;
    FVector LastOwnerVelocity = FVector::ZeroVector;

    // 当前注视方向（平滑插值用）
    FVector CurrentLookDir = FVector::ForwardVector;

    void UpdateSpineTilt(float DeltaTime);
    void UpdateHeadLookAt(float DeltaTime);
    void UpdateSpringMotion(float DeltaTime);
    void PushToAnimInstance();
};

// ─────────────────────────────────────────────────────────────
// 实现
// ─────────────────────────────────────────────────────────────

void UBoneModifierComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        MeshComp = OwnerCharacter->GetMesh();
        // AnimInst = Cast<UMyAnimInstance>(MeshComp->GetAnimInstance());
    }
}

void UBoneModifierComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!OwnerCharacter || !MeshComp) return;

    UpdateSpineTilt(DeltaTime);
    UpdateHeadLookAt(DeltaTime);
    UpdateSpringMotion(DeltaTime);
    PushToAnimInstance();
}

void UBoneModifierComponent::UpdateSpineTilt(float DeltaTime)
{
    // 获取控制器俯仰角
    AController* Controller = OwnerCharacter->GetController();
    if (!Controller) return;

    float AimPitch = Controller->GetControlRotation().Pitch;
    // Pitch 范围是 0~360，转换为 -180~180
    if (AimPitch > 180.0f) AimPitch -= 360.0f;

    // 计算目标脊椎倾斜
    float TargetPitch = FMath::Clamp(
        AimPitch * SpineTiltStrength,
        SpinePitchClamp.X,
        SpinePitchClamp.Y
    );

    // 平滑插值
    float CurrentPitch = SpineTiltRotation.Pitch;
    float NewPitch = FMath::FInterpTo(CurrentPitch, TargetPitch, DeltaTime, SpineTiltInterpSpeed);
    SpineTiltRotation = FRotator(NewPitch, 0.0f, 0.0f);
}

void UBoneModifierComponent::UpdateHeadLookAt(float DeltaTime)
{
    if (!bEnableHeadLookAt) return;

    FVector WorldTarget;

    if (LookAtTarget.IsValid())
    {
        // 注视指定 Actor（如：NPC 对话目标）
        WorldTarget = LookAtTarget->GetActorLocation();
    }
    else
    {
        // 注视相机前方 200cm 处
        AController* Controller = OwnerCharacter->GetController();
        if (!Controller) return;

        FVector CamLoc;
        FRotator CamRot;
        Controller->GetPlayerViewPoint(CamLoc, CamRot);
        WorldTarget = CamLoc + CamRot.Vector() * 200.0f;
    }

    // 转换为 Component Space
    FTransform CompWorld = MeshComp->GetComponentTransform();
    FVector TargetCS = CompWorld.InverseTransformPosition(WorldTarget);

    // 获取头部骨骼位置（Component Space）
    FVector HeadCS = MeshComp->GetBoneTransform(
        MeshComp->GetBoneIndex(FName("head"))).GetLocation();

    // 计算方向
    FVector NewDir = (TargetCS - HeadCS).GetSafeNormal();

    // 检查角度限制
    FVector CurrentFwd = OwnerCharacter->GetActorForwardVector();
    FVector CurrentFwdCS = CompWorld.InverseTransformVector(CurrentFwd).GetSafeNormal();
    float Angle = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(NewDir, CurrentFwdCS)));

    if (Angle > MaxHeadLookAngle)
    {
        // 超过角度，平滑归回
        NewDir = CurrentFwdCS;
    }

    // 平滑插值方向
    CurrentLookDir = FMath::VInterpTo(CurrentLookDir, NewDir, DeltaTime, HeadLookInterpSpeed);
    CurrentLookDir = CurrentLookDir.GetSafeNormal();

    // 计算目标点（从头部骨骼出发）
    HeadLookAtTargetCS = HeadCS + CurrentLookDir * 200.0f;
}

void UBoneModifierComponent::UpdateSpringMotion(float DeltaTime)
{
    // 简单弹簧求解（Hooke's Law + 阻尼）
    FVector CurrentVelocity = OwnerCharacter->GetVelocity();
    FVector Acceleration = (CurrentVelocity - LastOwnerVelocity) / FMath::Max(DeltaTime, 0.001f);
    LastOwnerVelocity = CurrentVelocity;

    // 弹簧力：-k * x（弹回原点）
    FVector SpringForce = -SpringStiffness * BackpackSpringOffset;
    // 阻尼力：-d * v
    FVector DampingForce = -SpringDamping * SpringVelocity;
    // 外部力（惯性反方向）
    FVector ExternalForce = -Acceleration * 0.05f;

    FVector TotalForce = SpringForce + DampingForce + ExternalForce;

    // 欧拉积分
    SpringVelocity += TotalForce * DeltaTime;
    BackpackSpringOffset += SpringVelocity * DeltaTime;

    // 限制最大偏移
    BackpackSpringOffset = BackpackSpringOffset.GetClampedToMaxSize(10.0f);
}

void UBoneModifierComponent::PushToAnimInstance()
{
    // 将计算结果推送到 AnimInstance
    // 注意：这在 Game Thread 中执行，而 AnimGraph 在 Worker Thread 中读取
    // 需要确保 AnimInstance 的这些变量是线程安全的（或使用 Property Access）

    if (!MeshComp) return;
    UAnimInstance* Anim = MeshComp->GetAnimInstance();
    if (!Anim) return;

    // 通过反射系统设置（通用方式，适合原型）：
    // 实际项目中应直接 Cast 到具体 AnimInstance 类
    // Cast<UMyAnimInstance>(Anim)->SpineTiltRotation = SpineTiltRotation;
    // Cast<UMyAnimInstance>(Anim)->HeadLookAtTargetCS = HeadLookAtTargetCS;
}

// =============================================================================
// 使用说明：
//
// 1. 在角色 BP 中添加 UBoneModifierComponent
// 2. 在 AnimBP 中添加对应的骨骼控制节点：
//    - Transform (Modify) Bone 节点读取 SpineTiltRotation
//    - Look At 节点读取 HeadLookAtTargetCS
//    - Spring Controller 节点读取 BackpackSpringOffset
// 3. 所有变量通过 AnimInstance 的 BlueprintReadOnly 属性连接
// =============================================================================
