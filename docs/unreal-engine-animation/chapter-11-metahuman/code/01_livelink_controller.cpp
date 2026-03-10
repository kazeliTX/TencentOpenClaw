// =============================================================================
// 01_livelink_controller.cpp
// Live Link 数据接收与 MetaHuman 面部驱动
// 对应文档：chapter-11-metahuman/04-live-link.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiveLinkTypes.h"
#include "ILiveLinkClient.h"
#include "LiveLinkController.generated.h"

UCLASS(ClassGroup=(MetaHuman), meta=(BlueprintSpawnableComponent))
class MYPROJECT_API ULiveLinkController : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="LiveLink")
    FName FaceSubjectName = FName("iPhone_Face");

    UPROPERTY(EditAnywhere, Category="LiveLink")
    float BlendShapeSmoothAlpha = 0.4f;  // 低通滤波系数

    // AnimInstance 读取此数组（52 个 BlendShape 值）
    UPROPERTY(BlueprintReadOnly, Category="LiveLink")
    TMap<FName, float> SmoothedBlendShapes;

    UPROPERTY(BlueprintReadOnly, Category="LiveLink")
    FRotator SmoothedHeadRotation;

    UPROPERTY(BlueprintReadOnly, Category="LiveLink")
    bool bIsConnected = false;

protected:
    virtual void TickComponent(float Dt, ELevelTick, FActorComponentTickFunction*) override
    {
        IModularFeatures& ModularFeatures = IModularFeatures::Get();
        if (!ModularFeatures.IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
            return;

        ILiveLinkClient& Client = ModularFeatures.GetModularFeature<ILiveLinkClient>(
            ILiveLinkClient::ModularFeatureName);

        FLiveLinkSubjectFrameData FrameData;
        bool bSuccess = Client.EvaluateFrame_AnyThread(
            FLiveLinkSubjectKey(FGuid(), FaceSubjectName),
            ULiveLinkAnimationRole::StaticClass(),
            FrameData);

        bIsConnected = bSuccess;
        if (!bSuccess) return;

        // 读取 BlendShape 曲线
        const FLiveLinkBaseFrameData* BaseData = FrameData.FrameData.Cast<FLiveLinkBaseFrameData>();
        if (!BaseData) return;

        for (int32 i = 0; i < BaseData->PropertyValues.Num(); i++)
        {
            FName CurveName = FrameData.StaticData.Cast<FLiveLinkBaseStaticData>()
                ->PropertyNames[i];
            float RawValue  = BaseData->PropertyValues[i];

            // 低通滤波（减少抖动）
            float Prev  = SmoothedBlendShapes.FindOrAdd(CurveName, 0.f);
            float Threshold = 0.02f;
            float Filtered  = (FMath::Abs(RawValue) < Threshold) ? 0.f : RawValue;
            SmoothedBlendShapes[CurveName] = FMath::Lerp(Prev, Filtered, BlendShapeSmoothAlpha);
        }
    }
};
