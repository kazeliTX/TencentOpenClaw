// =============================================================================
// 01_physics_rope.cpp
// 物理绳索链条：生成/附加/断裂/玩家攀爬
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "PhysicsRopeActor.generated.h"

UCLASS()
class APhysicsRopeActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) TSubclassOf<AActor> RopeNodeClass;
    UPROPERTY(EditAnywhere) int32  Segments       = 12;
    UPROPERTY(EditAnywhere) float  BreakForce      = 8000.f;
    UPROPERTY(EditAnywhere) float  SwingLimitDeg   = 25.f;

    TArray<UPhysicsConstraintComponent*> Constraints;
    TArray<AActor*> Nodes;

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        SpawnRope(GetActorLocation(),
            GetActorLocation() + FVector(0, 0, -300.f));
    }

    void SpawnRope(FVector Top, FVector Bottom)
    {
        FVector Step = (Bottom - Top) / Segments;
        Nodes.Empty();
        Constraints.Empty();

        for (int32 i = 0; i <= Segments; i++)
        {
            FVector Loc = Top + Step * i;
            AActor* Node = GetWorld()->SpawnActor<AActor>(
                RopeNodeClass, Loc, FRotator::ZeroRotator);
            if (!Node) continue;
            Nodes.Add(Node);

            if (i == 0)
            {
                // 顶端固定
                if (UPrimitiveComponent* PC = Cast<UPrimitiveComponent>(
                    Node->GetComponentByClass(UPrimitiveComponent::StaticClass())))
                    PC->SetSimulatePhysics(false);
            }

            if (i > 0 && Nodes.IsValidIndex(i-1))
            {
                // 创建约束组件
                UPhysicsConstraintComponent* PCC =
                    NewObject<UPhysicsConstraintComponent>(this);
                PCC->RegisterComponent();
                PCC->AttachToComponent(RootComponent,
                    FAttachmentTransformRules::KeepWorldTransform);
                PCC->SetWorldLocation(Loc - Step * 0.5f);

                PCC->ConstraintActor1 = Nodes[i-1];
                PCC->ConstraintActor2 = Nodes[i];

                // 允许摆动，锁定其他
                PCC->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Limited, SwingLimitDeg);
                PCC->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, SwingLimitDeg);
                PCC->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.f);
                PCC->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
                PCC->SetLinearYLimit(ELinearConstraintMotion::LCM_Locked, 0.f);
                PCC->SetLinearZLimit(ELinearConstraintMotion::LCM_Locked, 0.f);

                // 设置断裂阈值
                PCC->ConstraintInstance.SetLinearBreakable(true, BreakForce);
                PCC->OnConstraintBroken.AddDynamic(
                    this, &APhysicsRopeActor::OnConstraintBroken);

                Constraints.Add(PCC);
            }
        }
        UE_LOG(LogTemp, Log, TEXT("Spawned rope: %d nodes, %d constraints"),
            Nodes.Num(), Constraints.Num());
    }

    UFUNCTION()
    void OnConstraintBroken(int32 ConstraintIndex)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("Rope constraint broken at index %d"), ConstraintIndex);
        // 触发断绳效果（粒子/音效）
    }
};
