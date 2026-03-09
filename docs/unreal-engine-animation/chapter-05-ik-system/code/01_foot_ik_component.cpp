// =============================================================================
// 01_foot_ik_component.cpp
// 完整脚步 IK 组件（地形自适应）
// 对应文档：chapter-05-ik-system/04-foot-ik.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootIKComponent.generated.h"

USTRUCT(BlueprintType)
struct FFootIKResult
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) FVector  Location   = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FRotator Rotation   = FRotator::ZeroRotator;
    UPROPERTY(BlueprintReadOnly) bool     bGroundHit = false;
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UFootIKComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFootIKComponent() { PrimaryComponentTick.bCanEverTick = true; }

    // ── 对外暴露（AnimInstance 读取这些）──
    UPROPERTY(BlueprintReadOnly, Category="FootIK") FVector  LeftFootIKTarget;
    UPROPERTY(BlueprintReadOnly, Category="FootIK") FVector  RightFootIKTarget;
    UPROPERTY(BlueprintReadOnly, Category="FootIK") FRotator LeftFootRotation;
    UPROPERTY(BlueprintReadOnly, Category="FootIK") FRotator RightFootRotation;
    UPROPERTY(BlueprintReadOnly, Category="FootIK") float    PelvisOffset    = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="FootIK") float    FootIKAlpha     = 1.f;

    // ── 可调参数 ──
    UPROPERTY(EditAnywhere, Category="FootIK") FName LeftFootBone  = FName("foot_l");
    UPROPERTY(EditAnywhere, Category="FootIK") FName RightFootBone = FName("foot_r");
    UPROPERTY(EditAnywhere, Category="FootIK") float TraceUp   = 50.f;
    UPROPERTY(EditAnywhere, Category="FootIK") float TraceDown = 75.f;
    UPROPERTY(EditAnywhere, Category="FootIK") float FootZOffset = 5.f;
    UPROPERTY(EditAnywhere, Category="FootIK") float InterpSpeed = 15.f;
    UPROPERTY(EditAnywhere, Category="FootIK") float MaxPelvisDown = -20.f;

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        Owner = Cast<ACharacter>(GetOwner());
        if (Owner) Mesh = Owner->GetMesh();
    }

    virtual void TickComponent(float Dt, ELevelTick, FActorComponentTickFunction*) override
    {
        if (!Owner || !Mesh) return;

        // 速度驱动 IK 权重
        float Speed = Owner->GetVelocity().Size2D();
        float Target = (Speed < 10.f) ? 1.f : (Speed < 300.f) ? .7f : .3f;
        FootIKAlpha = FMath::FInterpTo(FootIKAlpha, Target, Dt, 5.f);

        FFootIKResult L = TraceAndCompute(LeftFootBone,  Dt);
        FFootIKResult R = TraceAndCompute(RightFootBone, Dt);

        // 骨盆偏移（取最低脚）
        float LDz = L.bGroundHit ? L.Location.Z : 0.f;
        float RDz = R.bGroundHit ? R.Location.Z : 0.f;
        float TargetPelvis = FMath::Clamp(FMath::Min(LDz, RDz), MaxPelvisDown, 0.f);
        PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, Dt, 8.f);

        if (L.bGroundHit) { LeftFootIKTarget  = L.Location; LeftFootRotation  = L.Rotation; }
        if (R.bGroundHit) { RightFootIKTarget = R.Location; RightFootRotation = R.Rotation; }
    }

private:
    FFootIKResult TraceAndCompute(FName FootBone, float Dt)
    {
        FFootIKResult Res;
        FVector FootWS = Mesh->GetBoneLocation(FootBone);
        FHitResult Hit;
        FCollisionQueryParams P; P.AddIgnoredActor(Owner);

        if (!Mesh->GetWorld()->LineTraceSingleByChannel(
            Hit,
            FootWS + FVector(0,0,TraceUp),
            FootWS - FVector(0,0,TraceDown),
            ECC_Visibility, P))
            return Res;

        Res.bGroundHit = true;
        // Component Space
        FVector WS = Hit.ImpactPoint + FVector(0,0,FootZOffset);
        Res.Location = Mesh->GetComponentTransform().InverseTransformPosition(WS);

        // 贴地旋转
        FVector N = Hit.ImpactNormal;
        FVector Fwd = Owner->GetActorForwardVector();
        Fwd = (Fwd - N * (Fwd | N)).GetSafeNormal();
        Res.Rotation = FMatrix(Fwd, FVector::CrossProduct(N,Fwd), N, {}).Rotator();
        return Res;
    }

    UPROPERTY() TObjectPtr<ACharacter>             Owner;
    UPROPERTY() TObjectPtr<USkeletalMeshComponent> Mesh;
};
