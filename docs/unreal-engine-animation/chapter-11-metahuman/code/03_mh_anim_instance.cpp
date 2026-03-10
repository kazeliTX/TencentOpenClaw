// =============================================================================
// 03_mh_anim_instance.cpp
// MetaHuman AnimInstance 扩展（身体 + 面部统一管理）
// 对应文档：chapter-11-metahuman/02-body-animation.md
// =============================================================================
#pragma once
#include "Animation/AnimInstance.h"
#include "MHAnimInstance.generated.h"

UCLASS()
class MYPROJECT_API UMHAnimInstance : public UAnimInstance
{
    GENERATED_BODY()
public:
    // 身体动画变量（AnimGraph Fast Path）
    UPROPERTY(BlueprintReadOnly) float  Speed       = 0.f;
    UPROPERTY(BlueprintReadOnly) float  Direction   = 0.f;
    UPROPERTY(BlueprintReadOnly) float  AimPitch    = 0.f;
    UPROPERTY(BlueprintReadOnly) bool   bIsInAir    = false;
    UPROPERTY(BlueprintReadOnly) bool   bIsCrouching= false;

    // 面部动画控制
    UPROPERTY(EditAnywhere, Category="Face") bool  bEnableFaceAnim    = true;
    UPROPERTY(EditAnywhere, Category="Face") bool  bUseLiveLink       = false;
    UPROPERTY(EditAnywhere, Category="Face") FName LiveLinkSubject    = "iPhone_Face";

    // 获取面部 AnimInstance（从 Face SkeletalMeshComponent）
    UFUNCTION(BlueprintCallable, Category="Face")
    UAnimInstance* GetFaceAnimInstance() const
    {
        APawn* Owner = TryGetPawnOwner();
        if (!Owner) return nullptr;
        USkeletalMeshComponent* FaceMesh = Owner->FindComponentByTag<USkeletalMeshComponent>(
            FName("Face"));
        return FaceMesh ? FaceMesh->GetAnimInstance() : nullptr;
    }

    // 程序化设置面部表情（通过 Face AnimInstance）
    UFUNCTION(BlueprintCallable, Category="Face")
    void SetFaceExpression(const TMap<FName, float>& BlendShapeValues)
    {
        if (UAnimInstance* FaceAnim = GetFaceAnimInstance())
        {
            for (auto& [Name, Value] : BlendShapeValues)
                FaceAnim->SetCurveValue(Name, Value);
        }
    }

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override
    {
        ACharacter* C = Cast<ACharacter>(TryGetPawnOwner());
        if (!C) return;

        Speed       = C->GetVelocity().Size2D();
        bIsInAir    = C->GetCharacterMovement()->IsFalling();
        bIsCrouching= C->GetCharacterMovement()->IsCrouching();

        if (AController* PC = C->GetController())
        {
            float Pitch = PC->GetControlRotation().Pitch;
            if (Pitch > 180.f) Pitch -= 360.f;
            AimPitch = FMath::Clamp(Pitch, -90.f, 90.f);
        }

        // 计算方向（Strafe）
        FVector NormVel = C->GetVelocity().GetSafeNormal2D();
        if (!NormVel.IsNearlyZero())
            Direction = FMath::RadiansToDegrees(FMath::Atan2(
                FVector::DotProduct(NormVel, C->GetActorRightVector()),
                FVector::DotProduct(NormVel, C->GetActorForwardVector())));
        else
            Direction = 0.f;
    }
};
