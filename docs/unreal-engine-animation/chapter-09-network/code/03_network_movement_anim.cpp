// =============================================================================
// 03_network_movement_anim.cpp
// 网络移动状态同步 + 驱动动画系统
// 对应文档：chapter-09-network/02-movement-replication.md
// =============================================================================
#pragma once

// Character.h 片段
// UPROPERTY(Replicated, BlueprintReadOnly) bool  bIsSprinting;
// UPROPERTY(Replicated, BlueprintReadOnly) float AimPitchCompressed; // 0~255
// UPROPERTY(Replicated, BlueprintReadOnly) uint8 WeaponType;

// Character.cpp 实现

void AMyCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyCharacter, bIsSprinting);
    DOREPLIFETIME_CONDITION(AMyCharacter, AimPitchCompressed, COND_SimulatedOnly);
    DOREPLIFETIME(AMyCharacter, WeaponType);
}

void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // 只在服务端更新 Replicated 变量
    if (HasAuthority())
    {
        // 压缩 AimPitch 为 uint8（1字节），节省带宽
        if (AController* PC = GetController())
        {
            float Pitch = PC->GetControlRotation().Pitch;
            if (Pitch > 180.f) Pitch -= 360.f;
            // 映射 -90~90 到 0~255
            AimPitchCompressed = (uint8)FMath::GetMappedRangeValueClamped(
                FVector2D(-90.f, 90.f), FVector2D(0.f, 255.f), Pitch);
        }
    }
}

// 解压 AimPitch（在 AnimInstance 中使用）
float GetAimPitchDegrees(const AMyCharacter* C)
{
    if (C->IsLocallyControlled())
    {
        AController* PC = C->GetController();
        if (PC)
        {
            float P = PC->GetControlRotation().Pitch;
            if (P > 180.f) P -= 360.f;
            return FMath::Clamp(P, -90.f, 90.f);
        }
    }
    // SimProxy：解压
    return FMath::GetMappedRangeValueClamped(
        FVector2D(0.f, 255.f), FVector2D(-90.f, 90.f),
        (float)C->AimPitchCompressed);
}
