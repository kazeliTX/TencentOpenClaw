// =============================================================================
// 02_buoyancy_system.cpp
// 浮力系统：多浮点计算/船只漂浮/随波浪倾斜
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BuoyancyComponent.h"
#include "BoatPawn.generated.h"

UCLASS()
class ABoatPawn : public APawn
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent*  BoatMesh;
    UPROPERTY(VisibleAnywhere) UBuoyancyComponent*    Buoyancy;

    ABoatPawn()
    {
        BoatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
        BoatMesh->SetSimulatePhysics(true);
        BoatMesh->SetLinearDamping(0.5f);
        BoatMesh->SetAngularDamping(2.0f);
        RootComponent = BoatMesh;

        Buoyancy = CreateDefaultSubobject<UBuoyancyComponent>(TEXT("Buoyancy"));
    }

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        SetupBuoyancyPontoons();
    }

    // 配置四角浮点
    void SetupBuoyancyPontoons()
    {
        // 需要在 Details 或代码中配置 Pontoons
        // 四个角的局部偏移（根据实际船只尺寸调整）
        TArray<FVector> PontoonOffsets = {
            FVector( 200.f,  100.f, -30.f),  // 前左
            FVector( 200.f, -100.f, -30.f),  // 前右
            FVector(-200.f,  100.f, -30.f),  // 后左
            FVector(-200.f, -100.f, -30.f),  // 后右
        };

        for (const FVector& Offset : PontoonOffsets)
        {
            FSphericalPontoon Pontoon;
            Pontoon.CenterSocket     = NAME_None;
            Pontoon.LocalOffset      = Offset;
            Pontoon.Radius           = 50.f;
            Pontoon.FXEnabled        = true;
            Buoyancy->Pontoons.Add(Pontoon);
        }

        // 浮力参数
        Buoyancy->BuoyancyData.BuoyancyCoefficient = 1.1f;  // 略微上浮
        Buoyancy->BuoyancyData.BuoyancyDamp        = 3.0f;
        Buoyancy->BuoyancyData.BuoyancyDampForSwimming = 5.0f;
        Buoyancy->BuoyancyData.BuoyancyRampMinVelocity = 10.f;
        Buoyancy->BuoyancyData.BuoyancyRampMaxVelocity = 400.f;
        Buoyancy->BuoyancyData.BuoyancyRampMax         = 1.0f;
    }

    // 船只推进（基于物理推力）
    UFUNCTION(BlueprintCallable)
    void Throttle(float ForwardInput, float TurnInput)
    {
        if (!BoatMesh->IsSimulatingPhysics()) return;

        float ThrottleForce = 500000.f * ForwardInput;
        float TurnTorque    = 200000.f * TurnInput;

        BoatMesh->AddForce(GetActorForwardVector() * ThrottleForce);
        BoatMesh->AddTorqueInDegrees(FVector(0, 0, TurnTorque));
    }
};
