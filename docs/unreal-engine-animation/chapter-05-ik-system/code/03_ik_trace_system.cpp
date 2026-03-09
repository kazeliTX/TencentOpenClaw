// =============================================================================
// 03_ik_trace_system.cpp
// IK 地面检测系统（带频率控制和缓存）
// 对应文档：chapter-05-ik-system/10-ik-optimization.md
// =============================================================================
#pragma once

/**
 * 优化的 IK 地面检测系统
 * 特性：
 *   - 频率控制（每 N 帧检测一次）
 *   - 结果缓存（静止时重用）
 *   - 多脚支持
 */
class FIKTraceSystem
{
public:
    struct FTraceConfig
    {
        FName  BoneName;
        float  TraceUp     = 50.f;
        float  TraceDown   = 75.f;
        float  FootZOffset = 5.f;
        int32  TraceEveryNFrames = 2;  // 每 2 帧检测一次
    };

    struct FTraceResult
    {
        FVector  IKTarget  = FVector::ZeroVector;
        FRotator IKRotation= FRotator::ZeroRotator;
        bool     bValid    = false;
        int32    LastFrame = -1;
    };

    static FTraceResult Trace(
        UWorld* World,
        USkeletalMeshComponent* Mesh,
        AActor* IgnoreActor,
        const FTraceConfig& Config,
        FTraceResult& CachedResult,
        int32 CurrentFrame)
    {
        // 频率控制：未到检测帧则返回缓存
        if (CurrentFrame - CachedResult.LastFrame < Config.TraceEveryNFrames)
            return CachedResult;

        FVector FootWS = Mesh->GetBoneLocation(Config.BoneName);
        FHitResult Hit;
        FCollisionQueryParams P(FName("FootIK"), false, IgnoreActor);

        bool bHit = World->LineTraceSingleByChannel(
            Hit,
            FootWS + FVector(0,0,Config.TraceUp),
            FootWS - FVector(0,0,Config.TraceDown),
            ECC_Visibility, P);

        if (!bHit)
        {
            CachedResult.bValid    = false;
            CachedResult.LastFrame = CurrentFrame;
            return CachedResult;
        }

        FVector WS = Hit.ImpactPoint + FVector(0,0,Config.FootZOffset);
        CachedResult.IKTarget   = Mesh->GetComponentTransform().InverseTransformPosition(WS);
        FVector N   = Hit.ImpactNormal;
        FVector Fwd = Mesh->GetForwardVector();
        Fwd = (Fwd - N*(Fwd|N)).GetSafeNormal();
        CachedResult.IKRotation = FMatrix(Fwd, FVector::CrossProduct(N,Fwd), N, {}).Rotator();
        CachedResult.bValid     = true;
        CachedResult.LastFrame  = CurrentFrame;
        return CachedResult;
    }
};
