// =============================================================================
// 01_fracture_trigger.cpp
// 运行时触发破碎：子弹命中/爆炸/累积伤害 + 破碎事件回调
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemActor.h"
#include "FractureTriggerDemo.generated.h"

UCLASS()
class ADestructibleWall : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) UGeometryCollectionComponent* GCC;
    UPROPERTY(EditAnywhere, Category="FX") UParticleSystem* BreakVFX;
    UPROPERTY(EditAnywhere, Category="FX") USoundBase*      BreakSFX;
    UPROPERTY(EditAnywhere, Category="FX") USoundBase*      HeavyBreakSFX;

    // 当前累积伤害（用于 Single damage model）
    float AccumulatedStrain = 0.f;
    UPROPERTY(EditAnywhere) float BreakStrainThreshold = 1500.f;

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        GCC->OnChaosBreakEvent.AddDynamic(this, &ADestructibleWall::OnBreak);
        GCC->OnChaosRemovalEvent.AddDynamic(this, &ADestructibleWall::OnRemoval);
    }

    // ─────────────────────────────────────────────
    // 子弹命中（局部破碎）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable)
    void OnBulletHit(FVector HitLocation, float ImpactForce)
    {
        ApplyStrainAtLocation(HitLocation, ImpactForce, 25.f); // 半径 25cm
    }

    // ─────────────────────────────────────────────
    // 爆炸（大范围破碎）
    // ─────────────────────────────────────────────
    UFUNCTION(BlueprintCallable)
    void OnExplosion(FVector ExplosionCenter, float Radius, float Strain)
    {
        ApplyStrainAtLocation(ExplosionCenter, Strain, Radius);
    }

    // ─────────────────────────────────────────────
    // 核心：在指定点施加应变 Field
    // ─────────────────────────────────────────────
    void ApplyStrainAtLocation(FVector Center, float StrainMag, float Radius)
    {
        FActorSpawnParameters P;
        P.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AFieldSystemActor* FA = GetWorld()->SpawnActor<AFieldSystemActor>(
            AFieldSystemActor::StaticClass(), Center, FRotator::ZeroRotator, P);
        if (!FA) return;

        UFieldSystemComponent* FSC = FA->GetFieldSystemComponent();
        
        // 径向应变场（从 Center 向外 Radius 内，线性衰减）
        FSC->ApplyLinearForce(
            false,
            EFieldPhysicsType::Field_ExternalClusterStrain,
            FFieldSystemMetaDataFilter(),
            Center, StrainMag, Radius,
            EFieldFalloffType::Field_FallOff_Linear);

        FA->SetLifeSpan(0.05f);

        // 播放音效
        if (BreakSFX)
            UGameplayStatics::PlaySoundAtLocation(this, BreakSFX, Center);
    }

    // ─────────────────────────────────────────────
    // 破碎事件回调
    // ─────────────────────────────────────────────
    UFUNCTION()
    void OnBreak(const FChaosBreakEvent& Ev)
    {
        if (BreakVFX)
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), BreakVFX, Ev.Location);

        float Speed = Ev.Velocity.Size();
        USoundBase* SFX = (Speed > 500.f && HeavyBreakSFX) ?
            HeavyBreakSFX : BreakSFX;
        if (SFX)
            UGameplayStatics::PlaySoundAtLocation(this, SFX, Ev.Location);

        UE_LOG(LogTemp, Log, TEXT("Break: loc=%s vel=%.0f mass=%.1f"),
            *Ev.Location.ToString(), Speed, Ev.Mass);
    }

    UFUNCTION()
    void OnRemoval(const FChaosRemovalEvent& Ev)
    {
        // 微小碎片消失时生成粉尘
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), BreakVFX, Ev.Location, FRotator::ZeroRotator,
            FVector(0.3f));  // 缩小特效尺寸
    }
};
