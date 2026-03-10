// =============================================================================
// 03_custom_trace_channel.cpp
// 自定义 Trace Channel 查询：武器检测、视线检测、相机避障
// 对应文档：chapter-02-collision-system/07-trace-channels.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CustomTraceChannelDemo.generated.h"

UCLASS()
class ACustomTraceChannelDemo : public AActor
{
    GENERATED_BODY()
public:
    // ─────────────────────────────────────────────
    // 1. 近战武器扫掠（SphereTrace + WeaponTrace 通道）
    // ─────────────────────────────────────────────
    TArray<FHitResult> MeleeWeaponTrace(
        FVector Start, FVector End, float Radius = 25.f)
    {
        TArray<FHitResult> Results;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.bTraceComplex = false;
        Params.bReturnPhysicalMaterial = true;

        // ECC_GameTraceChannel1 = WeaponTrace（DefaultEngine.ini 中定义）
        bool bHit = GetWorld()->SweepMultiByChannel(
            Results, Start, End, FQuat::Identity,
            ECC_GameTraceChannel1,
            FCollisionShape::MakeSphere(Radius),
            Params);

        // 调试显示
        DrawDebugCylinder(GetWorld(), Start, End, Radius, 8,
            bHit ? FColor::Red : FColor::Green, false, 1.f);

        // 去重（同 Actor 的多个骨骼可能都被命中）
        TSet<AActor*> Seen;
        TArray<FHitResult> Deduped;
        for (const FHitResult& H : Results)
        {
            if (AActor* A = H.GetActor())
            {
                if (!Seen.Contains(A))
                {
                    Seen.Add(A);
                    Deduped.Add(H);
                }
            }
        }
        return Deduped;
    }

    // ─────────────────────────────────────────────
    // 2. 视线检测（LineTrace + Visibility 通道）
    // ─────────────────────────────────────────────
    bool HasLineOfSight(AActor* Target) const
    {
        if (!Target) return false;
        FVector Start = GetActorLocation() + FVector(0,0,60.f);
        FVector End   = Target->GetActorLocation() + FVector(0,0,60.f);

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(Target);  // 忽略目标自身，只检测中间遮挡物

        FHitResult Hit;
        bool bBlocked = GetWorld()->LineTraceSingleByChannel(
            Hit, Start, End, ECC_Visibility, Params);

        if (bBlocked)
        {
            DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint,
                FColor::Red, false, 0.1f, 0, 1.f);
            return false;  // 被遮挡
        }
        DrawDebugLine(GetWorld(), Start, End,
            FColor::Green, false, 0.1f, 0, 1.f);
        return true;
    }

    // ─────────────────────────────────────────────
    // 3. 地面检测（LineTrace 向下，找地面法线）
    //    用于 Foot IK、斜面检测
    // ─────────────────────────────────────────────
    bool TraceGround(FVector Location, FVector& OutHitPoint, FVector& OutHitNormal)
    {
        FVector Start = Location + FVector(0,0,50.f);
        FVector End   = Location - FVector(0,0,200.f);

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.bTraceComplex = true;  // 地面用 Complex 获取精确法线
        Params.bReturnPhysicalMaterial = true;

        FHitResult Hit;
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit, Start, End, ECC_Visibility, Params);

        if (bHit)
        {
            OutHitPoint  = Hit.ImpactPoint;
            OutHitNormal = Hit.ImpactNormal;
            // 斜面角度
            float SlopeAngle = FMath::RadiansToDegrees(
                FMath::Acos(FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector)));
            UE_LOG(LogTemp, Verbose, TEXT("Ground slope: %.1f°"), SlopeAngle);
        }
        return bHit;
    }

    // ─────────────────────────────────────────────
    // 4. 按对象类型查询（找场景中所有 Pawn）
    // ─────────────────────────────────────────────
    TArray<AActor*> FindAllPawnsInRange(float Radius)
    {
        TArray<FOverlapResult> OverlapResults;
        FCollisionObjectQueryParams ObjParams;
        ObjParams.AddObjectTypesToQuery(ECC_Pawn);

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        GetWorld()->OverlapMultiByObjectType(
            OverlapResults,
            GetActorLocation(),
            FQuat::Identity,
            ObjParams,
            FCollisionShape::MakeSphere(Radius),
            Params);

        TArray<AActor*> Actors;
        for (const FOverlapResult& R : OverlapResults)
        {
            if (AActor* A = R.GetActor())
                Actors.AddUnique(A);
        }

        DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 16,
            FColor::Yellow, false, 1.f);
        UE_LOG(LogTemp, Log, TEXT("Found %d Pawns in range %.0f cm"),
            Actors.Num(), Radius);
        return Actors;
    }

    // ─────────────────────────────────────────────
    // 5. 异步 Trace（不阻塞游戏线程）
    // ─────────────────────────────────────────────
    void AsyncTraceExample(FVector Start, FVector End)
    {
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        // 发起异步追踪
        FTraceHandle Handle = GetWorld()->AsyncLineTraceByChannel(
            EAsyncTraceType::Single,
            Start, End,
            ECC_Visibility, Params);

        // 保存 Handle，在下帧查询结果
        // 实际项目中应存为成员变量
        FTraceDatum Datum;
        if (GetWorld()->QueryTraceData(Handle, Datum))
        {
            if (Datum.OutHits.Num() > 0)
            {
                const FHitResult& Hit = Datum.OutHits[0];
                UE_LOG(LogTemp, Log, TEXT("AsyncTrace Hit: %s at dist=%.0f"),
                    Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("none"),
                    Hit.Distance);
            }
        }
    }

    // ─────────────────────────────────────────────
    // 6. 最近阻挡点查询（用于技能范围预测）
    // ─────────────────────────────────────────────
    FVector FindFirstBlockingPoint(FVector Start, FVector Direction, float MaxRange)
    {
        FVector End = Start + Direction.GetSafeNormal() * MaxRange;
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
            return Hit.ImpactPoint;
        return End;  // 没有遮挡，返回最远点
    }
};
