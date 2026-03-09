// =============================================================================
// 02_trajectory_generator.cpp
// 角色轨迹预测生成器（用于 Motion Matching 查询）
// 对应文档：chapter-06-motion-matching/06-trajectory-prediction.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrajectoryGenerator.generated.h"

USTRUCT(BlueprintType)
struct FTrajectoryPoint
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FVector  Position;
    UPROPERTY(BlueprintReadOnly) FRotator Facing;
    UPROPERTY(BlueprintReadOnly) float    Time = 0.f;
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UTrajectoryGenerator : public UActorComponent
{
    GENERATED_BODY()
public:
    UTrajectoryGenerator() { PrimaryComponentTick.bCanEverTick = true; }

    UPROPERTY(EditAnywhere, Category="Trajectory") int32  HistoryCount    = 6;
    UPROPERTY(EditAnywhere, Category="Trajectory") int32  PredictionCount = 6;
    UPROPERTY(EditAnywhere, Category="Trajectory") float  SampleInterval  = 0.1f;
    UPROPERTY(EditAnywhere, Category="Trajectory") float  SpringStiffness = 10.f;

    UPROPERTY(BlueprintReadOnly, Category="Trajectory")
    TArray<FTrajectoryPoint> TrajectoryPoints; // 历史（负时间）+ 未来（正时间）

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        Owner = Cast<ACharacter>(GetOwner());
    }

    virtual void TickComponent(float Dt, ELevelTick, FActorComponentTickFunction*) override
    {
        if (!Owner) return;
        UpdateHistory(Dt);
        PredictFuture();
    }

private:
    void UpdateHistory(float Dt)
    {
        // 记录当前位置到历史
        FTrajectoryPoint Current;
        Current.Position = Owner->GetActorLocation();
        Current.Facing   = Owner->GetActorRotation();
        Current.Time     = 0.f;
        HistoryBuffer.Insert(Current, 0);
        if (HistoryBuffer.Num() > HistoryCount + 1)
            HistoryBuffer.SetNum(HistoryCount + 1);
    }

    void PredictFuture()
    {
        TrajectoryPoints.Reset();

        // 添加历史点（负时间）
        for (int32 i = FMath::Min(HistoryBuffer.Num() - 1, HistoryCount - 1); i >= 1; i--)
        {
            FTrajectoryPoint P = HistoryBuffer[i];
            P.Time = -(i * SampleInterval);
            TrajectoryPoints.Add(P);
        }

        // 当前点（t=0）
        FTrajectoryPoint Now;
        Now.Position = Owner->GetActorLocation();
        Now.Facing   = Owner->GetActorRotation();
        Now.Time     = 0.f;
        TrajectoryPoints.Add(Now);

        // 预测未来（SpringDamper 简化）
        FVector  PredPos = Now.Position;
        FVector  PredVel = Owner->GetVelocity();
        FRotator PredRot = Now.Facing;
        FVector  TargetVel = GetDesiredVelocity();

        for (int32 i = 1; i <= PredictionCount; i++)
        {
            float T = SampleInterval;
            PredVel = FMath::VInterpTo(PredVel, TargetVel, T, SpringStiffness);
            PredPos += PredVel * T;

            if (!PredVel.IsNearlyZero())
                PredRot = FMath::RInterpTo(PredRot, PredVel.Rotation(), T, 8.f);

            FTrajectoryPoint P;
            P.Position = PredPos;
            P.Facing   = PredRot;
            P.Time     = i * SampleInterval;
            TrajectoryPoints.Add(P);
        }
    }

    FVector GetDesiredVelocity() const
    {
        if (!Owner) return FVector::ZeroVector;
        UCharacterMovementComponent* CMC = Owner->GetCharacterMovement();
        if (!CMC) return FVector::ZeroVector;
        return CMC->GetLastInputVector() * CMC->MaxWalkSpeed;
    }

    UPROPERTY() TObjectPtr<ACharacter> Owner;
    TArray<FTrajectoryPoint>           HistoryBuffer;
};
