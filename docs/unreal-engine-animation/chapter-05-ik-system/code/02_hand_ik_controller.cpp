// =============================================================================
// 02_hand_ik_controller.cpp
// 手部双手持枪 IK 控制器
// 对应文档：chapter-05-ik-system/05-hand-ik.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HandIKController.generated.h"

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UHandIKController : public UActorComponent
{
    GENERATED_BODY()
public:
    UHandIKController() { PrimaryComponentTick.bCanEverTick = true; }

    // ── AnimInstance 读取 ──
    UPROPERTY(BlueprintReadOnly, Category="HandIK") FVector  LeftHandIKTarget;
    UPROPERTY(BlueprintReadOnly, Category="HandIK") FRotator LeftHandIKRotation;
    UPROPERTY(BlueprintReadOnly, Category="HandIK") float    LeftHandIKAlpha = 0.f;

    // ── 控制接口 ──
    UFUNCTION(BlueprintCallable) void SetWeapon(AActor* NewWeapon, FName LeftHandSocket)
    {
        CurrentWeapon = NewWeapon;
        LeftHandSocketName = LeftHandSocket;
        bIKEnabled = (NewWeapon != nullptr);
    }

    UFUNCTION(BlueprintCallable) void SetIKEnabled(bool bEnabled)
    {
        bIKEnabled = bEnabled;
    }

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        if (ACharacter* C = Cast<ACharacter>(GetOwner())) Mesh = C->GetMesh();
    }

    virtual void TickComponent(float Dt, ELevelTick, FActorComponentTickFunction*) override
    {
        if (!Mesh) return;

        // Alpha 平滑
        float TargetAlpha = bIKEnabled ? 1.f : 0.f;
        LeftHandIKAlpha = FMath::FInterpTo(LeftHandIKAlpha, TargetAlpha, Dt, 10.f);

        if (!bIKEnabled || !CurrentWeapon || LeftHandIKAlpha < 0.01f) return;

        // 找左手 Socket（武器上）
        USkeletalMeshComponent* WeaponMesh =
            CurrentWeapon->FindComponentByClass<USkeletalMeshComponent>();
        if (!WeaponMesh) return;

        FTransform SocketWS = WeaponMesh->GetSocketTransform(LeftHandSocketName);
        FTransform CompInv  = Mesh->GetComponentTransform().Inverse();
        FTransform SocketCS = SocketWS * CompInv;

        LeftHandIKTarget = FMath::VInterpTo(
            LeftHandIKTarget, SocketCS.GetLocation(), Dt, 20.f);
        LeftHandIKRotation = FMath::RInterpTo(
            LeftHandIKRotation, SocketCS.GetRotation().Rotator(), Dt, 20.f);
    }

private:
    UPROPERTY() TObjectPtr<USkeletalMeshComponent> Mesh;
    UPROPERTY() TObjectPtr<AActor>                 CurrentWeapon;
    FName LeftHandSocketName = FName("LeftHandSocket");
    bool  bIKEnabled = false;
};
