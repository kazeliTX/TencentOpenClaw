// =============================================================================
// 03_lag_compensation.cpp
// 延迟补偿：快照历史 + 时间回滚命中检测
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "LagCompensationManager.generated.h"

USTRUCT()
struct FActorSnapshot
{
    GENERATED_BODY()
    float   Timestamp = 0.f;
    FVector Location  = FVector::ZeroVector;
    FQuat   Rotation  = FQuat::Identity;
    FVector Extent    = FVector(50.f); // AABB 半径（用于快速命中检测）
};

USTRUCT()
struct FActorHistory
{
    GENERATED_BODY()
    TWeakObjectPtr<AActor> Actor;
    TArray<FActorSnapshot> Snapshots; // 最新在前
};

UCLASS()
class ALagCompensationManager : public AActor
{
    GENERATED_BODY()
public:
    static const int32 MaxHistory    = 120; // 2秒（60fps）
    static const float MaxCompWindow = 1.5f; // 最大补偿窗口（秒）

    TArray<FActorHistory> Histories;

    // ─── 每帧记录所有需要补偿的 Actor 快照 ───
    void RecordFrame()
    {
        float Now = GetWorld()->GetTimeSeconds();
        for (FActorHistory& H : Histories)
        {
            if (!H.Actor.IsValid()) continue;
            FActorSnapshot S;
            S.Timestamp = Now;
            S.Location  = H.Actor->GetActorLocation();
            S.Rotation  = H.Actor->GetActorQuat();
            H.Snapshots.Insert(S, 0);
            if (H.Snapshots.Num() > MaxHistory)
                H.Snapshots.SetNum(MaxHistory);
        }
    }

    // ─── 延迟补偿命中检测 ───
    // ShooterRTT: 射手的往返延迟（ms）
    bool CompensatedLineTrace(
        const FVector& Start, const FVector& End,
        float ShooterRTT, FHitResult& OutHit)
    {
        float TargetTime = GetWorld()->GetTimeSeconds()
            - FMath::Clamp(ShooterRTT * 0.001f, 0.f, MaxCompWindow);

        for (FActorHistory& H : Histories)
        {
            if (!H.Actor.IsValid()) continue;
            FActorSnapshot Snap = GetSnapshotAtTime(H, TargetTime);

            // 临时移动 Actor 到历史位置（回滚）
            FVector OriginalLoc = H.Actor->GetActorLocation();
            FQuat   OriginalRot = H.Actor->GetActorQuat();
            H.Actor->SetActorLocationAndRotation(Snap.Location,
                Snap.Rotation, false, nullptr, ETeleportType::TeleportPhysics);

            // 执行 Trace（针对回滚状态）
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(H.Actor.Get());
            bool bHit = GetWorld()->LineTraceSingleByChannel(
                OutHit, Start, End, ECC_GameTraceChannel1, Params);

            // 恢复位置
            H.Actor->SetActorLocationAndRotation(OriginalLoc,
                OriginalRot, false, nullptr, ETeleportType::TeleportPhysics);

            if (bHit) return true;
        }
        return false;
    }

    FActorSnapshot GetSnapshotAtTime(
        const FActorHistory& H, float TargetTime)
    {
        for (int32 i = 0; i + 1 < H.Snapshots.Num(); i++)
        {
            const FActorSnapshot& A = H.Snapshots[i+1];
            const FActorSnapshot& B = H.Snapshots[i];
            if (A.Timestamp <= TargetTime && TargetTime <= B.Timestamp)
            {
                float Alpha = (TargetTime - A.Timestamp) /
                    FMath::Max(B.Timestamp - A.Timestamp, 0.0001f);
                FActorSnapshot R;
                R.Timestamp = TargetTime;
                R.Location  = FMath::Lerp(A.Location, B.Location, Alpha);
                R.Rotation  = FQuat::Slerp(A.Rotation, B.Rotation, Alpha);
                return R;
            }
        }
        return H.Snapshots.IsEmpty() ? FActorSnapshot{} : H.Snapshots.Last();
    }
};
