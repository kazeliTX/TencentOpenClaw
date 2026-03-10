// =============================================================================
// 02_physics_query.cpp
// 物理查询：LineTrace/SphereTrace/BoxTrace/OverlapMulti/AsyncTrace
// 对应文档：chapter-01-physics-fundamentals/10-debug-visualization.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Async/Async.h"
#include "PhysicsQueryActor.generated.h"

UCLASS()
class MYPROJECT_API APhysicsQueryActor : public AActor
{
    GENERATED_BODY()
public:
    // ─────────────────────────────────────────────────────────────────
    // 1. LineTrace（射线检测）
    // ─────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="PhysicsQuery")
    bool LineTraceExample(FVector Start, FVector End)
    {
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);         // 忽略自身
        Params.bTraceComplex = false;          // 用 Simple Collision（更快）
        Params.bReturnPhysicalMaterial = true; // 返回物理材质
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start, End,
            ECC_Visibility,  // 碰撞通道：Visibility
            Params
        );
        
        if (bHit)
        {
            // HitResult 包含详细碰撞信息
            UE_LOG(LogTemp, Log, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());
            UE_LOG(LogTemp, Log, TEXT("  Impact Point: %s"), *HitResult.ImpactPoint.ToString());
            UE_LOG(LogTemp, Log, TEXT("  Impact Normal: %s"), *HitResult.ImpactNormal.ToString());
            UE_LOG(LogTemp, Log, TEXT("  Distance: %.2f cm"), HitResult.Distance);
            UE_LOG(LogTemp, Log, TEXT("  Bone Name: %s"), *HitResult.BoneName.ToString());
            
            if (HitResult.PhysMaterial.IsValid())
                UE_LOG(LogTemp, Log, TEXT("  PhysMat: %s"), *HitResult.PhysMaterial->GetName());
            
            // 调试绘制
            DrawDebugLine(GetWorld(), Start, HitResult.ImpactPoint, FColor::Green, false, 3.f, 0, 1.f);
            DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.f, FColor::Red, false, 3.f);
            DrawDebugLine(GetWorld(), HitResult.ImpactPoint,
                HitResult.ImpactPoint + HitResult.ImpactNormal * 30.f,
                FColor::Blue, false, 3.f, 0, 1.f);
        }
        else
        {
            DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 3.f, 0, 1.f);
        }
        
        return bHit;
    }

    // ─────────────────────────────────────────────────────────────────
    // 2. LineTrace Multi（射线多目标检测）
    // ─────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="PhysicsQuery")
    TArray<AActor*> LineTraceMultiExample(FVector Start, FVector End)
    {
        TArray<FHitResult> HitResults;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.bTraceComplex = false;
        
        bool bHit = GetWorld()->LineTraceMultiByChannel(
            HitResults, Start, End, ECC_Visibility, Params);
        
        TArray<AActor*> HitActors;
        for (const FHitResult& Hit : HitResults)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                HitActors.AddUnique(HitActor);
                DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 8.f, FColor::Orange, false, 3.f);
            }
        }
        UE_LOG(LogTemp, Log, TEXT("LineTrace Multi hit %d objects"), HitResults.Num());
        return HitActors;
    }

    // ─────────────────────────────────────────────────────────────────
    // 3. SphereTrace（球形扫掠检测）
    // ─────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="PhysicsQuery")
    bool SphereTraceExample(FVector Start, FVector End, float Radius = 30.f)
    {
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        
        bool bHit = GetWorld()->SweepSingleByChannel(
            HitResult,
            Start, End,
            FQuat::Identity,
            ECC_PhysicsBody,
            FCollisionShape::MakeSphere(Radius),  // 球形
            Params
        );
        
        if (bHit)
        {
            DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, Radius, 12, FColor::Yellow, false, 3.f);
            UE_LOG(LogTemp, Log, TEXT("SphereTrace Hit: %s at %s"),
                *HitResult.GetActor()->GetName(), *HitResult.ImpactPoint.ToString());
        }
        DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 3.f, 0, 1.f);
        return bHit;
    }

    // ─────────────────────────────────────────────────────────────────
    // 4. BoxTrace（盒形扫掠检测）
    // ─────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="PhysicsQuery")
    TArray<FHitResult> BoxTraceExample(FVector Start, FVector End, FVector HalfExtent)
    {
        TArray<FHitResult> HitResults;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        
        GetWorld()->SweepMultiByChannel(
            HitResults,
            Start, End,
            FQuat::Identity,
            ECC_PhysicsBody,
            FCollisionShape::MakeBox(HalfExtent),  // 盒形
            Params
        );
        
        DrawDebugBox(GetWorld(), (Start+End)/2, HalfExtent, FColor::Cyan, false, 3.f);
        UE_LOG(LogTemp, Log, TEXT("BoxTrace found %d hits"), HitResults.Num());
        return HitResults;
    }

    // ─────────────────────────────────────────────────────────────────
    // 5. OverlapMulti（范围重叠检测）
    // ─────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="PhysicsQuery")
    TArray<AActor*> SphereOverlapActors(FVector Center, float Radius)
    {
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        
        GetWorld()->OverlapMultiByObjectType(
            OverlapResults,
            Center,
            FQuat::Identity,
            ObjectParams,
            FCollisionShape::MakeSphere(Radius),
            Params
        );
        
        TArray<AActor*> Actors;
        for (const FOverlapResult& Overlap : OverlapResults)
        {
            if (AActor* A = Overlap.GetActor())
                Actors.AddUnique(A);
        }
        
        DrawDebugSphere(GetWorld(), Center, Radius, 16, FColor::Purple, false, 3.f);
        UE_LOG(LogTemp, Log, TEXT("SphereOverlap found %d actors in radius %.0f cm"),
            Actors.Num(), Radius);
        return Actors;
    }

    // ─────────────────────────────────────────────────────────────────
    // 6. 异步 LineTrace（不阻塞游戏线程，回调结果）
    // ─────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="PhysicsQuery")
    void AsyncLineTraceExample(FVector Start, FVector End)
    {
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        
        // 发起异步 Trace（立即返回，不阻塞）
        FTraceHandle Handle = GetWorld()->AsyncLineTraceByChannel(
            EAsyncTraceType::Single,   // Single / Multi
            Start, End,
            ECC_Visibility,
            Params
        );
        
        // 下一帧读取结果（也可以存 Handle 延后读）
        FLatentActionInfo LatentInfo;
        GetWorld()->QueryTraceData(Handle, [this](const FTraceDatum& Data)
        {
            if (Data.OutHits.Num() > 0)
            {
                const FHitResult& Hit = Data.OutHits[0];
                UE_LOG(LogTemp, Log, TEXT("Async Hit: %s"),
                    Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("none"));
            }
        });
    }

    // ─────────────────────────────────────────────────────────────────
    // 7. 自定义碰撞 Profile 查询
    // ─────────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category="PhysicsQuery")
    bool LineTraceByProfile(FVector Start, FVector End, FName ProfileName)
    {
        FHitResult HitResult;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.bTraceComplex = false;
        
        // 使用 Collision Profile 查询（而非单一通道）
        bool bHit = GetWorld()->LineTraceSingleByProfile(
            HitResult, Start, End, ProfileName, Params);
        
        UE_LOG(LogTemp, Log, TEXT("ProfileTrace [%s]: %s"),
            *ProfileName.ToString(), bHit ? TEXT("HIT") : TEXT("MISS"));
        return bHit;
    }
};
