// =============================================================================
// 02_face_anim_driver.cpp
// 程序化面部动画驱动（表情系统 + 眨眼 + 注视）
// 对应文档：chapter-11-metahuman/06-control-rig-face.md
// =============================================================================
#pragma once
#include "Animation/AnimInstance.h"
#include "FaceAnimDriver.generated.h"

UENUM(BlueprintType)
enum class EFaceEmotion : uint8 { Neutral, Happy, Sad, Angry, Surprised, Disgusted };

UCLASS()
class MYPROJECT_API UFaceAnimDriver : public UAnimInstance
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Face") float BlinkMinInterval = 2.f;
    UPROPERTY(EditAnywhere, Category="Face") float BlinkMaxInterval = 6.f;
    UPROPERTY(EditAnywhere, Category="Face") float BlinkDuration    = 0.1f;
    UPROPERTY(EditAnywhere, Category="Face") float EmotionBlendSpeed= 3.f;

    UFUNCTION(BlueprintCallable) void SetEmotion(EFaceEmotion Emotion)
    { TargetEmotion = Emotion; }
    UFUNCTION(BlueprintCallable) void SetLookAtTarget(FVector WorldPos)
    { LookAtWorldPosition = WorldPos; bHasLookAt = true; }

protected:
    virtual void NativeBeginPlay() override
    {
        Super::NativeBeginPlay();
        ScheduleNextBlink();
    }

    virtual void NativeUpdateAnimation(float DeltaSeconds) override
    {
        UpdateBlink(DeltaSeconds);
        UpdateEmotion(DeltaSeconds);
        UpdateLookAt(DeltaSeconds);
        FlushCurves();
    }

private:
    // --- 状态 ---
    EFaceEmotion TargetEmotion  = EFaceEmotion::Neutral;
    FVector      LookAtWorldPosition;
    bool         bHasLookAt     = false;

    // --- 眨眼 ---
    float  BlinkTimer     = 0.f;
    float  BlinkInterval  = 3.f;
    float  BlinkProgress  = 0.f; // 0=open, 1=closed
    bool   bBlinking      = false;

    // --- 当前插值中的 BlendShape 值 ---
    TMap<FName, float> CurrentCurves;

    void ScheduleNextBlink()
    {
        BlinkInterval = FMath::FRandRange(BlinkMinInterval, BlinkMaxInterval);
        BlinkTimer    = 0.f;
    }

    void UpdateBlink(float Dt)
    {
        BlinkTimer += Dt;
        if (!bBlinking && BlinkTimer >= BlinkInterval)
        {
            bBlinking    = true;
            BlinkProgress = 0.f;
        }
        if (bBlinking)
        {
            BlinkProgress += Dt / BlinkDuration;
            if (BlinkProgress >= 1.f) { bBlinking = false; ScheduleNextBlink(); }
        }
        float BlinkValue = bBlinking ? FMath::Sin(BlinkProgress * PI) : 0.f;
        // 随机让左右眼略微不同步（自然感）
        CurrentCurves.FindOrAdd(FName("eyeBlink_L")) = BlinkValue;
        CurrentCurves.FindOrAdd(FName("eyeBlink_R")) = BlinkValue * FMath::FRandRange(0.9f, 1.0f);
    }

    void UpdateEmotion(float Dt)
    {
        TMap<FName, float> TargetCurves;
        switch (TargetEmotion)
        {
        case EFaceEmotion::Happy:
            TargetCurves.Add("mouthSmile_L", 0.8f); TargetCurves.Add("mouthSmile_R", 0.8f);
            TargetCurves.Add("cheekSquint_L", 0.5f); TargetCurves.Add("cheekSquint_R", 0.5f);
            break;
        case EFaceEmotion::Angry:
            TargetCurves.Add("browDown_L", 0.8f); TargetCurves.Add("browDown_R", 0.8f);
            TargetCurves.Add("mouthFrown_L", 0.6f); TargetCurves.Add("mouthFrown_R", 0.6f);
            break;
        case EFaceEmotion::Sad:
            TargetCurves.Add("browInnerUp", 0.5f);
            TargetCurves.Add("mouthFrown_L", 0.4f); TargetCurves.Add("mouthFrown_R", 0.4f);
            break;
        case EFaceEmotion::Surprised:
            TargetCurves.Add("browInnerUp", 0.8f);
            TargetCurves.Add("eyeWide_L", 0.7f); TargetCurves.Add("eyeWide_R", 0.7f);
            TargetCurves.Add("jawOpen", 0.3f);
            break;
        default: break; // Neutral：所有值为 0
        }

        // 对所有 emotion 相关曲线插值
        static TArray<FName> EmotionCurveNames = {
            "mouthSmile_L","mouthSmile_R","cheekSquint_L","cheekSquint_R",
            "browDown_L","browDown_R","mouthFrown_L","mouthFrown_R",
            "browInnerUp","eyeWide_L","eyeWide_R","jawOpen"
        };
        for (const FName& N : EmotionCurveNames)
        {
            float Target = TargetCurves.FindOrAdd(N, 0.f);
            float& Curr  = CurrentCurves.FindOrAdd(N, 0.f);
            Curr = FMath::FInterpTo(Curr, Target, Dt, EmotionBlendSpeed);
        }
    }

    void UpdateLookAt(float Dt)
    {
        if (!bHasLookAt) return;
        APawn* Owner = TryGetPawnOwner();
        if (!Owner) return;
        FVector ToTarget = (LookAtWorldPosition - Owner->GetActorLocation()).GetSafeNormal();
        FVector Forward  = Owner->GetActorForwardVector();
        float   Up       = FVector::DotProduct(ToTarget, FVector::UpVector);
        float   Right    = FVector::DotProduct(ToTarget, Owner->GetActorRightVector());
        CurrentCurves.FindOrAdd("eyeLookUp_L")    = FMath::Clamp( Up,    0.f, 1.f);
        CurrentCurves.FindOrAdd("eyeLookDown_L")  = FMath::Clamp(-Up,    0.f, 1.f);
        CurrentCurves.FindOrAdd("eyeLookIn_L")    = FMath::Clamp(-Right, 0.f, 1.f);
        CurrentCurves.FindOrAdd("eyeLookOut_L")   = FMath::Clamp( Right, 0.f, 1.f);
        CurrentCurves.FindOrAdd("eyeLookUp_R")    = CurrentCurves["eyeLookUp_L"];
        CurrentCurves.FindOrAdd("eyeLookDown_R")  = CurrentCurves["eyeLookDown_L"];
        CurrentCurves.FindOrAdd("eyeLookIn_R")    = CurrentCurves["eyeLookIn_L"];
        CurrentCurves.FindOrAdd("eyeLookOut_R")   = CurrentCurves["eyeLookOut_L"];
    }

    void FlushCurves()
    {
        for (auto& [Name, Value] : CurrentCurves)
            SetCurveValue(Name, Value);
    }
};
