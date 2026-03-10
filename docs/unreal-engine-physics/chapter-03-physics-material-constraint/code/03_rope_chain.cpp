// =============================================================================
// 03_rope_chain.cpp
// 物理绳索/链条运行时创建：弹簧链约束、可断裂绳索、CableComponent 对比
// 对应文档：chapter-03/10-rope-chain.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "CableComponent.h"
#include "RopeChainDemo.generated.h"

UCLASS()
class APhysicsRope : public AActor
{
    GENERATED_BODY()
public:
    // 绳索分段 Mesh 列表
    UPROPERTY()
    TArray<UStaticMeshComponent*> RopeSegments;
    UPROPERTY()
    TArray<UPhysicsConstraintComponent*> RopeConstraints;

    UPROPERTY(EditAnywhere, Category="Rope") int32  NumSegments    = 12;
    UPROPERTY(EditAnywhere, Category="Rope") float  SegmentLength  = 20.f;   // cm
    UPROPERTY(EditAnywhere, Category="Rope") float  SegmentMass    = 0.5f;   // kg
    UPROPERTY(EditAnywhere, Category="Rope") float  LinearStiff    = 5000.f; // 拉伸刚度
    UPROPERTY(EditAnywhere, Category="Rope") float  AngularStiff   = 100.f;  // 弯曲刚度
    UPROPERTY(EditAnywhere, Category="Rope") float  Damping        = 30.f;
    UPROPERTY(EditAnywhere, Category="Rope") bool   bBreakable     = true;
    UPROPERTY(EditAnywhere, Category="Rope") float  BreakForce     = 50000.f; // N

    UPROPERTY(EditAnywhere, Category="Rope") UStaticMesh* SegmentMesh; // 小胶囊/圆柱Mesh

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        BuildRope();
    }

    // ─────────────────────────────────────────────
    // 构建物理绳索
    // ─────────────────────────────────────────────
    void BuildRope()
    {
        RopeSegments.Empty();
        RopeConstraints.Empty();

        UStaticMeshComponent* PrevSegment = nullptr;

        for (int32 i = 0; i < NumSegments; ++i)
        {
            // 1. 创建绳段 Component
            FName SegName = *FString::Printf(TEXT("Seg_%02d"), i);
            UStaticMeshComponent* Seg = NewObject<UStaticMeshComponent>(this, SegName);
            Seg->RegisterComponent();
            Seg->SetStaticMesh(SegmentMesh);
            Seg->SetWorldLocation(
                GetActorLocation() + FVector(0, 0, -i * SegmentLength));

            if (i == 0)
            {
                // 第一段固定（绳子顶端）
                Seg->SetSimulatePhysics(false);
                Seg->SetMobility(EComponentMobility::Static);
                Seg->AttachToComponent(RootComponent,
                    FAttachmentTransformRules::KeepWorldTransform);
            }
            else
            {
                // 其余段参与物理
                Seg->SetSimulatePhysics(true);
                Seg->SetMassOverrideInKg(NAME_None, SegmentMass);
                Seg->SetLinearDamping(0.5f);
                Seg->SetAngularDamping(1.0f);
                Seg->SetCollisionProfileName(FName("PhysicsActor"));
                Seg->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }

            RopeSegments.Add(Seg);

            // 2. 创建约束（连接到上一段）
            if (PrevSegment && i > 0)
            {
                FName ConName = *FString::Printf(TEXT("Con_%02d"), i);
                UPhysicsConstraintComponent* Con =
                    NewObject<UPhysicsConstraintComponent>(this, ConName);
                Con->RegisterComponent();
                Con->SetWorldLocation(
                    GetActorLocation() + FVector(0, 0, -(i - 0.5f) * SegmentLength));

                // 绑定两段
                Con->SetConstrainedComponents(PrevSegment, NAME_None, Seg, NAME_None);

                // 线性：微弹性（允许轻微拉伸）
                Con->SetLinearXLimit(LCM_Limited, 2.f);
                Con->SetLinearYLimit(LCM_Limited, 2.f);
                Con->SetLinearZLimit(LCM_Limited, 2.f);

                // 线性软限制（弹簧感）
                FConstraintInstance& CI = Con->ConstraintInstance;
                CI.ProfileInstance.LinearLimit.bSoftConstraint = true;
                CI.ProfileInstance.LinearLimit.Stiffness = LinearStiff;
                CI.ProfileInstance.LinearLimit.Damping   = Damping;

                // 角度：有限摆动（绳子弯曲限制）
                Con->SetAngularSwing1Limit(ACM_Limited, 45.f);
                Con->SetAngularSwing2Limit(ACM_Limited, 45.f);
                Con->SetAngularTwistLimit(ACM_Free, 0.f); // 扭转自由

                // 角度软限制
                CI.ProfileInstance.ConeLimit.bSoftConstraint = true;
                CI.ProfileInstance.ConeLimit.Stiffness = AngularStiff;
                CI.ProfileInstance.ConeLimit.Damping   = Damping * 0.5f;

                // 可断裂设置
                if (bBreakable)
                {
                    CI.ProfileInstance.LinearLimit.bBreakable = true;
                    CI.ProfileInstance.LinearLimit.BreakForce = BreakForce;
                    CI.ProfileInstance.ConeLimit.bBreakable    = true;
                    CI.ProfileInstance.ConeLimit.BreakTorque   = BreakForce * 10.f;
                    Con->OnConstraintBroken.AddDynamic(
                        this, &APhysicsRope::OnRopeSegmentBroken);
                }

                Con->UpdateConstraintFrames();
                RopeConstraints.Add(Con);
            }

            PrevSegment = Seg;
        }

        UE_LOG(LogTemp, Log, TEXT("Rope built: %d segments, %d constraints"),
            RopeSegments.Num(), RopeConstraints.Num());
    }

    // ─────────────────────────────────────────────
    // 绳索断裂回调
    // ─────────────────────────────────────────────
    UFUNCTION()
    void OnRopeSegmentBroken(int32 ConstraintIndex)
    {
        UE_LOG(LogTemp, Warning, TEXT("Rope broken at segment %d!"), ConstraintIndex);
        // 播放断裂音效/粒子
        // 断裂后面的绳段自由下落（已经脱离约束，Chaos 自动处理）
    }

    // ─────────────────────────────────────────────
    // 运行时调整绳索刚度（如绳子被浸湿变软）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Rope")
    void SetRopeStiffness(float NewLinearStiff, float NewAngularStiff)
    {
        for (UPhysicsConstraintComponent* Con : RopeConstraints)
        {
            if (!Con) continue;
            FConstraintInstance& CI = Con->ConstraintInstance;
            CI.ProfileInstance.LinearLimit.Stiffness = NewLinearStiff;
            CI.ProfileInstance.ConeLimit.Stiffness   = NewAngularStiff;
            Con->UpdateConstraintFrames();
        }
    }

    // ─────────────────────────────────────────────
    // 施加风力（对所有绳段施加横向力）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Rope")
    void ApplyWind(FVector WindForce)
    {
        for (UStaticMeshComponent* Seg : RopeSegments)
        {
            if (Seg && Seg->IsSimulatingPhysics())
                Seg->AddForce(WindForce);
        }
    }

    // ─────────────────────────────────────────────
    // 在绳尾挂接重物
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="Rope")
    void AttachPayload(UStaticMeshComponent* Payload, float PayloadMass)
    {
        if (RopeSegments.Num() == 0 || !Payload) return;

        UStaticMeshComponent* TailSeg = RopeSegments.Last();
        Payload->SetSimulatePhysics(true);
        Payload->SetMassOverrideInKg(NAME_None, PayloadMass);

        UPhysicsConstraintComponent* AttachCon =
            NewObject<UPhysicsConstraintComponent>(this, TEXT("PayloadCon"));
        AttachCon->RegisterComponent();
        AttachCon->SetWorldLocation(TailSeg->GetComponentLocation());
        AttachCon->SetConstrainedComponents(TailSeg, NAME_None, Payload, NAME_None);

        // 固定约束（绳尾焊接重物）
        AttachCon->SetLinearXLimit(LCM_Locked, 0);
        AttachCon->SetLinearYLimit(LCM_Locked, 0);
        AttachCon->SetLinearZLimit(LCM_Locked, 0);
        AttachCon->SetAngularSwing1Limit(ACM_Locked, 0);
        AttachCon->SetAngularSwing2Limit(ACM_Locked, 0);
        AttachCon->SetAngularTwistLimit(ACM_Locked, 0);
        AttachCon->UpdateConstraintFrames();
        RopeConstraints.Add(AttachCon);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// 视觉绳索（UCableComponent 快速使用示例）
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class AVisualRopeActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UCableComponent* Cable;
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* AttachPoint; // 绳尾挂点

    AVisualRopeActor()
    {
        RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

        Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
        Cable->SetupAttachment(RootComponent);
        Cable->CableLength       = 300.f;  // 绳子总长 300 cm
        Cable->NumSegments       = 20;     // 分段数
        Cable->SolverIterations  = 8;      // XPBD 迭代（越高越稳定）
        Cable->SubstepTime       = 0.005f; // 子步长
        Cable->CableGravityScale = 1.0f;
        Cable->bEnableStiffness  = false;  // 不启用刚度（更柔软）
        Cable->bAttachStart      = true;
        Cable->bAttachEnd        = true;   // 两端都固定

        AttachPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachPoint"));
        AttachPoint->SetupAttachment(RootComponent);
        AttachPoint->SetRelativeLocation(FVector(0, 200, 0)); // 绳子终点

        // 绳尾接到 AttachPoint
        Cable->SetAttachEndToComponent(AttachPoint);
    }

    // 动态改变绳长（如放绳/收绳）
    UFUNCTION(BlueprintCallable)
    void SetCableLength(float Length)
    {
        Cable->CableLength = FMath::Clamp(Length, 50.f, 1000.f);
    }
};
