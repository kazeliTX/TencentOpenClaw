// =============================================================================
// 01_montage_manager.cpp
// Montage 播放管理器（支持队列、优先级、连击）
// 对应文档：chapter-04-blend-montage/04-animation-montage.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "MontageManager.generated.h"

/** Montage 播放请求 */
USTRUCT(BlueprintType)
struct FMontagePlayRequest
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) TObjectPtr<UAnimMontage> Montage;
    UPROPERTY(EditAnywhere) FName StartSection = NAME_None;
    UPROPERTY(EditAnywhere) float PlayRate     = 1.0f;
    UPROPERTY(EditAnywhere) int32 Priority     = 0;        // 高优先级打断低优先级
    UPROPERTY(EditAnywhere) float BlendInTime  = -1.0f;    // -1 = 使用 Montage 默认值
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMontageCompleted,
    UAnimMontage*, Montage, bool, bInterrupted);

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API UMontageManager : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable) FOnMontageCompleted OnMontageCompleted;

    /** 播放 Montage（自动处理优先级和打断）*/
    UFUNCTION(BlueprintCallable, Category="Montage")
    bool PlayMontage(const FMontagePlayRequest& Request)
    {
        if (!CachedAnim || !Request.Montage) return false;

        // 检查优先级：新请求优先级 >= 当前才允许打断
        if (CurrentPriority > Request.Priority && CachedAnim->IsAnyMontagePlaying())
        {
            UE_LOG(LogTemp, Log, TEXT("Montage blocked by priority: %d vs %d"),
                Request.Priority, CurrentPriority);
            return false;
        }

        CurrentPriority = Request.Priority;

        // 停止当前 Montage（如果有）
        if (CurrentMontage && CachedAnim->Montage_IsActive(CurrentMontage))
            CachedAnim->Montage_Stop(0.15f, CurrentMontage);

        // 播放新 Montage
        float Duration = CachedAnim->Montage_Play(
            Request.Montage, Request.PlayRate,
            EMontagePlayReturnType::MontageLength,
            0.0f, true);

        if (Duration <= 0.0f) return false;

        CurrentMontage = Request.Montage;

        // 跳到指定 Section
        if (Request.StartSection != NAME_None)
            CachedAnim->Montage_JumpToSection(Request.StartSection, Request.Montage);

        // 注册结束回调
        FOnMontageEnded EndDel;
        EndDel.BindUObject(this, &UMontageManager::HandleMontageEnded);
        CachedAnim->Montage_SetEndDelegate(EndDel, Request.Montage);

        return true;
    }

    /** 跳到 Section */
    UFUNCTION(BlueprintCallable, Category="Montage")
    void JumpToSection(FName Section, UAnimMontage* Montage = nullptr)
    {
        if (CachedAnim)
            CachedAnim->Montage_JumpToSection(Section, Montage ? Montage : CurrentMontage);
    }

    /** 停止当前 Montage */
    UFUNCTION(BlueprintCallable, Category="Montage")
    void StopCurrentMontage(float BlendOut = 0.2f)
    {
        if (CachedAnim) CachedAnim->Montage_Stop(BlendOut, CurrentMontage);
    }

    UFUNCTION(BlueprintPure) bool IsMontageActive() const
    {
        return CachedAnim && CurrentMontage
            && CachedAnim->Montage_IsActive(CurrentMontage);
    }

    UFUNCTION(BlueprintPure) FName GetCurrentSection() const
    {
        return (CachedAnim && CurrentMontage)
            ? CachedAnim->Montage_GetCurrentSection(CurrentMontage)
            : NAME_None;
    }

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        if (ACharacter* C = Cast<ACharacter>(GetOwner()))
            CachedAnim = C->GetMesh()->GetAnimInstance();
    }

private:
    void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
    {
        CurrentPriority = 0;
        CurrentMontage  = nullptr;
        OnMontageCompleted.Broadcast(Montage, bInterrupted);
    }

    UPROPERTY() TObjectPtr<UAnimInstance> CachedAnim;
    UPROPERTY() TObjectPtr<UAnimMontage>  CurrentMontage;
    int32 CurrentPriority = 0;
};
