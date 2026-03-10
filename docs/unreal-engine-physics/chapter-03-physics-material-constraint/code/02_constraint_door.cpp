// =============================================================================
// 02_constraint_door.cpp
// 铰链门完整实现：约束创建/角度限制/弹簧自动关门/电机驱动
// 对应文档：chapter-03/05-hinge-constraint.md & 09-constraint-motor.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "ConstraintDoorDemo.generated.h"

UENUM(BlueprintType)
enum class EDoorState : uint8
{
    Closed,
    Opening,
    Open,
    Closing
};

UCLASS()
class AConstraintDoor : public AActor
{
    GENERATED_BODY()
public:
    // 门框（固定，不模拟物理）
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* DoorFrame;
    // 门板（物理模拟）
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* DoorPanel;
    // 铰链约束
    UPROPERTY(VisibleAnywhere) UPhysicsConstraintComponent* HingeConstraint;

    // 参数
    UPROPERTY(EditAnywhere, Category="Door") float MaxOpenAngle  = 100.f; // 最大开门角度
    UPROPERTY(EditAnywhere, Category="Door") float AutoCloseDelay = 3.f;  // 多久后自动关
    UPROPERTY(EditAnywhere, Category="Door") float OpenSpeed     = 2000.f;// 开门驱动力
    UPROPERTY(EditAnywhere, Category="Door") float CloseStiffness = 200.f;// 关门弹簧刚度
    UPROPERTY(EditAnywhere, Category="Door") float CloseDamping   = 50.f; // 关门阻尼

    EDoorState State = EDoorState::Closed;
    FTimerHandle AutoCloseTimer;

    AConstraintDoor()
    {
        PrimaryActorTick.bCanEverTick = true;

        DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
        DoorFrame->SetSimulatePhysics(false);
        DoorFrame->SetMobility(EComponentMobility::Static);
        RootComponent = DoorFrame;

        DoorPanel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorPanel"));
        DoorPanel->SetupAttachment(DoorFrame);
        DoorPanel->SetSimulatePhysics(true);
        DoorPanel->SetMassOverrideInKg(NAME_None, 20.f);
        DoorPanel->SetLinearDamping(0.5f);
        DoorPanel->SetAngularDamping(3.0f);
        DoorPanel->SetNotifyRigidBodyCollision(true);

        HingeConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("Hinge"));
        HingeConstraint->SetupAttachment(DoorFrame);
        // 铰链位置 = 门框边缘（铰链安装点）
        HingeConstraint->SetRelativeLocation(FVector(0, -45, 0));
    }

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        SetupHingeConstraint();
        SetupAutoClose();

        // 响应角色推门
        DoorPanel->OnComponentHit.AddDynamic(this, &AConstraintDoor::OnDoorHit);
    }

    // ─────────────────────────────────────────────
    // 铰链约束配置
    // ─────────────────────────────────────────────
    void SetupHingeConstraint()
    {
        // 绑定两个 Component
        HingeConstraint->SetConstrainedComponents(
            DoorFrame, NAME_None,
            DoorPanel, NAME_None);

        // 线性：全锁（门板不平移）
        HingeConstraint->SetLinearXLimit(LCM_Locked, 0.f);
        HingeConstraint->SetLinearYLimit(LCM_Locked, 0.f);
        HingeConstraint->SetLinearZLimit(LCM_Locked, 0.f);

        // 角度：Z 轴（扭转轴）自由，X/Y 摆动锁定
        // 注意：UE 约束的 Twist = 绕 X 轴，但门绕 Z 旋转
        // → 需要旋转约束坐标系，使 X 轴朝上（对准门的旋转轴）
        HingeConstraint->SetRelativeRotation(FRotator(0, 0, 90.f));
        // 现在 Twist 轴 = 世界 Z 轴

        HingeConstraint->SetAngularTwistLimit(ACM_Limited, MaxOpenAngle);
        HingeConstraint->SetAngularSwing1Limit(ACM_Locked, 0.f);
        HingeConstraint->SetAngularSwing2Limit(ACM_Locked, 0.f);

        // 软限制（到达极限时弹性缓冲）
        FConstraintInstance& CI = HingeConstraint->ConstraintInstance;
        CI.ProfileInstance.TwistLimit.bSoftConstraint  = true;
        CI.ProfileInstance.TwistLimit.Stiffness        = 150.f;
        CI.ProfileInstance.TwistLimit.Damping          = 20.f;
        CI.ProfileInstance.TwistLimit.ContactDistance  = 5.f;

        HingeConstraint->UpdateConstraintFrames();
    }

    // ─────────────────────────────────────────────
    // 自动关门弹簧驱动
    // ─────────────────────────────────────────────
    void SetupAutoClose()
    {
        // 启用角度驱动：朝向驱动（目标 = 关闭角度）
        HingeConstraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
        HingeConstraint->SetAngularOrientationTarget(FRotator(0, 0, 0));
        HingeConstraint->SetAngularDriveParams(
            CloseStiffness,  // Position Strength（弹簧刚度）
            CloseDamping,    // Velocity Strength（阻尼）
            2000.f           // Max Force
        );
        // 初始先关闭驱动（等门被推开后再启动）
        HingeConstraint->SetOrientationDriveTwistAndSwing(false, false);
    }

    // ─────────────────────────────────────────────
    // 角色推门
    // ─────────────────────────────────────────────
    UFUNCTION()
    void OnDoorHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NI, const FHitResult& Hit)
    {
        if (State == EDoorState::Closed || State == EDoorState::Closing)
        {
            // 根据角色与门的相对位置决定推开方向
            FVector ToActor = (OtherActor->GetActorLocation()
                - GetActorLocation()).GetSafeNormal2D();
            FVector PushDir = FVector::CrossProduct(ToActor, FVector::UpVector);
            DoorPanel->AddImpulseAtLocation(
                PushDir * 50000.f, Hit.ImpactPoint);

            State = EDoorState::Opening;

            // 重置自动关门计时器
            GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimer);
            GetWorld()->GetTimerManager().SetTimer(AutoCloseTimer, this,
                &AConstraintDoor::StartAutoClose, AutoCloseDelay, false);
        }
    }

    // ─────────────────────────────────────────────
    // 启动自动关门
    // ─────────────────────────────────────────────
    void StartAutoClose()
    {
        State = EDoorState::Closing;
        // 打开角度朝向驱动，弹簧把门拉回关闭
        HingeConstraint->SetOrientationDriveTwistAndSwing(true, false);
        HingeConstraint->SetAngularDriveParams(
            CloseStiffness, CloseDamping, 2000.f);
    }

    virtual void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);
        if (State == EDoorState::Closing)
        {
            // 检测门是否已关闭（旋转角度接近 0）
            FQuat DoorQuat = DoorPanel->GetComponentQuat();
            FQuat FrameQuat = DoorFrame->GetComponentQuat();
            float AngleDiff = FMath::RadiansToDegrees(
                DoorQuat.AngularDistance(FrameQuat));

            if (AngleDiff < 2.f)
            {
                State = EDoorState::Closed;
                // 关门后停止驱动（节省性能）
                HingeConstraint->SetOrientationDriveTwistAndSwing(false, false);
                // 让门板进入 Sleep
                DoorPanel->PutRigidBodyToSleep();
            }
        }
    }

    // ─────────────────────────────────────────────
    // 强制打开/关闭（程序控制，如电动门）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Door")
    void SetDoorOpen(bool bOpen)
    {
        if (bOpen)
        {
            // 速度驱动：快速推开到目标角度
            HingeConstraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
            HingeConstraint->SetAngularVelocityTarget(FVector(0, 0, 3.f));
            HingeConstraint->SetVelocityDriveTwistAndSwing(true, false);
            HingeConstraint->SetAngularDriveParams(0.f, OpenSpeed, 100000.f);
            State = EDoorState::Opening;
        }
        else
        {
            StartAutoClose();
        }
    }
};
