// =============================================================================
// 01_root_motion_controller.cpp
// Root Motion 网络同步控制器
// 对应文档：chapter-06-motion-matching/03-root-motion-network.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RootMotionNetController.generated.h"

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API URootMotionNetController : public UActorComponent
{
    GENERATED_BODY()
public:
    /**
     * 播放 Root Motion Montage（支持网络同步）
     * 本地：立即预测播放
     * 服务器：权威执行并广播给其他客户端
     */
    UFUNCTION(BlueprintCallable, Category="RootMotion")
    void PlayRootMotionMontage(UAnimMontage* Montage, FVector InputDirection)
    {
        if (!Montage) return;

        ACharacter* Char = Cast<ACharacter>(GetOwner());
        if (!Char) return;

        // 朝向输入方向
        if (!InputDirection.IsNearlyZero())
        {
            FRotator NewRot = InputDirection.GetSafeNormal().Rotation();
            Char->SetActorRotation(NewRot);
        }

        // 本地立即播放（预测）
        Char->PlayAnimMontage(Montage);

        // 发给服务器
        if (!Char->HasAuthority())
            Server_PlayMontage(Montage, InputDirection);
    }

protected:
    UFUNCTION(Server, Reliable)
    void Server_PlayMontage(UAnimMontage* Montage, FVector InputDir)
    {
        ACharacter* Char = Cast<ACharacter>(GetOwner());
        if (!Char) return;

        if (!InputDir.IsNearlyZero())
            Char->SetActorRotation(InputDir.GetSafeNormal().Rotation());

        float Dur = Char->PlayAnimMontage(Montage);
        if (Dur > 0.f)
            Multicast_PlayMontage(Montage, InputDir);
    }

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayMontage(UAnimMontage* Montage, FVector InputDir)
    {
        ACharacter* Char = Cast<ACharacter>(GetOwner());
        if (!Char || Char->IsLocallyControlled()) return; // 本地已预测，跳过

        if (!InputDir.IsNearlyZero())
            Char->SetActorRotation(InputDir.GetSafeNormal().Rotation());

        Char->PlayAnimMontage(Montage);
    }
};
