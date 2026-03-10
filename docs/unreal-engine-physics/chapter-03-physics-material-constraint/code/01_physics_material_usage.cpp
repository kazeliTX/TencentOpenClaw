// =============================================================================
// 01_physics_material_usage.cpp
// 运行时物理材质创建与替换，SurfaceType 查询，脚步音效系统
// 对应文档：chapter-03/01-physics-material-overview.md & 03-surface-type.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/EngineTypes.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsMaterialUsageDemo.generated.h"

UCLASS()
class APhysicsMaterialDemo : public ACharacter
{
    GENERATED_BODY()
public:
    // 脚步音效映射表
    UPROPERTY(EditAnywhere, Category="Footstep")
    TMap<TEnumAsByte<EPhysicalSurface>, USoundBase*> FootstepSoundMap;

    // 弹孔贴花映射表
    UPROPERTY(EditAnywhere, Category="Impact")
    TMap<TEnumAsByte<EPhysicalSurface>, UMaterialInterface*> ImpactDecalMap;

    // 命中粒子映射表
    UPROPERTY(EditAnywhere, Category="Impact")
    TMap<TEnumAsByte<EPhysicalSurface>, UParticleSystem*> ImpactParticleMap;

    // ─────────────────────────────────────────────
    // 运行时创建物理材质（如 Runtime 生成 ProceduralMaterial）
    // ─────────────────────────────────────────────
    UPhysicalMaterial* CreatePhysicalMaterial(
        float Friction, float Restitution, float Density,
        EPhysicalSurface SurfaceType)
    {
        UPhysicalMaterial* PM = NewObject<UPhysicalMaterial>(this,
            UPhysicalMaterial::StaticClass(), NAME_None, RF_Transient);
        PM->Friction          = Friction;
        PM->StaticFriction    = Friction * 1.1f;  // 静摩擦略大于动摩擦
        PM->Restitution       = Restitution;
        PM->Density           = Density;
        PM->SurfaceType       = SurfaceType;
        PM->bOverrideFrictionCombineMode    = false;
        PM->bOverrideRestitutionCombineMode = false;
        return PM;
    }

    // ─────────────────────────────────────────────
    // 运行时替换 Component 的物理材质
    // ─────────────────────────────────────────────
    void SetComponentPhysicalMaterial(
        UPrimitiveComponent* Comp, UPhysicalMaterial* PhysMat)
    {
        if (!Comp || !PhysMat) return;
        // PhysMaterialOverride 覆盖 Mesh Material 中的物理材质
        Comp->SetPhysMaterialOverride(PhysMat);
        UE_LOG(LogTemp, Log, TEXT("PhysMat changed: Friction=%.2f Restitution=%.2f"),
            PhysMat->Friction, PhysMat->Restitution);
    }

    // ─────────────────────────────────────────────
    // 预设材质切换（冰面/正常/橡胶）
    // ─────────────────────────────────────────────
    enum class EGroundMaterial { Normal, Ice, Rubber };

    void SwitchGroundMaterial(UPrimitiveComponent* Ground, EGroundMaterial Type)
    {
        UPhysicalMaterial* PM = nullptr;
        switch (Type)
        {
            case EGroundMaterial::Ice:
            {
                PM = CreatePhysicalMaterial(0.02f, 0.05f, 0.9f, SurfaceType7);
                PM->FrictionCombineMode = EFrictionCombineMode::Min; // 冰面 Min 合并
                break;
            }
            case EGroundMaterial::Rubber:
            {
                PM = CreatePhysicalMaterial(0.9f, 0.7f, 1.1f, SurfaceType11);
                PM->FrictionCombineMode = EFrictionCombineMode::Max; // 橡胶 Max 合并
                break;
            }
            default:
            {
                PM = CreatePhysicalMaterial(0.7f, 0.3f, 1.0f, SurfaceType_Default);
                break;
            }
        }
        SetComponentPhysicalMaterial(Ground, PM);
    }

    // ─────────────────────────────────────────────
    // 脚步音效系统（在 AnimNotify 中调用）
    // ─────────────────────────────────────────────
    void PlayFootstepSound(bool bLeftFoot)
    {
        FName FootBone = bLeftFoot ? FName("foot_l") : FName("foot_r");
        FVector FootLoc = GetMesh()->GetBoneLocation(FootBone);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.bReturnPhysicalMaterial = true;

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit,
            FootLoc + FVector(0, 0, 15.f),
            FootLoc - FVector(0, 0, 60.f),
            ECC_Visibility, Params);

        if (!bHit) return;

        EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(
            Hit.PhysMaterial.Get());

        // 查表获取音效
        USoundBase* Sound = FootstepSoundMap.FindRef(Surface);
        if (!Sound) Sound = FootstepSoundMap.FindRef(SurfaceType_Default);
        if (!Sound) return;

        // 随机音调（防止重复感）
        float Pitch  = FMath::RandRange(0.9f, 1.1f);
        float Volume = 1.f;

        // 速度影响音量（奔跑比走路响）
        float Speed = GetVelocity().Size();
        Volume = FMath::GetMappedRangeValueClamped(
            FVector2D(0.f, 600.f), FVector2D(0.5f, 1.0f), Speed);

        UGameplayStatics::PlaySoundAtLocation(this, Sound, FootLoc, Volume, Pitch);

        // 泥地留下足迹
        if (Surface == SurfaceType4 || Surface == SurfaceType6) // Dirt or Sand
            SpawnFootprintDecal(FootLoc, GetActorRotation(), bLeftFoot);
    }

    // ─────────────────────────────────────────────
    // 足迹贴花（泥地/雪地）
    // ─────────────────────────────────────────────
    UPROPERTY(EditAnywhere, Category="Footstep")
    UMaterialInterface* FootprintDecalMat;

    void SpawnFootprintDecal(FVector Location, FRotator Rotation, bool bLeft)
    {
        if (!FootprintDecalMat) return;
        FRotator DecalRot = Rotation;
        DecalRot.Pitch = -90.f;  // 贴花垂直向下投影
        UGameplayStatics::SpawnDecalAtLocation(
            GetWorld(), FootprintDecalMat,
            FVector(15.f, 8.f, 0.2f),  // 贴花尺寸（脚印大小）
            Location + FVector(bLeft ? -5.f : 5.f, 0, 0),
            DecalRot, 15.f);  // 15 秒消失
    }

    // ─────────────────────────────────────────────
    // 命中效果（弹孔贴花 + 粒子）
    // ─────────────────────────────────────────────
    void SpawnImpactEffects(const FHitResult& Hit)
    {
        if (!Hit.PhysMaterial.IsValid()) return;
        EPhysicalSurface Surface = Hit.PhysMaterial->SurfaceType;

        // 贴花
        UMaterialInterface* Decal = ImpactDecalMap.FindRef(Surface);
        if (!Decal) Decal = ImpactDecalMap.FindRef(SurfaceType_Default);
        if (Decal)
        {
            FRotator DecalRot = Hit.ImpactNormal.Rotation();
            UGameplayStatics::SpawnDecalAtLocation(
                GetWorld(), Decal, FVector(5, 5, 0.1f),
                Hit.ImpactPoint, DecalRot, 10.f);
        }

        // 粒子
        UParticleSystem* Particle = ImpactParticleMap.FindRef(Surface);
        if (!Particle) Particle = ImpactParticleMap.FindRef(SurfaceType_Default);
        if (Particle)
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), Particle,
                Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
    }

    // ─────────────────────────────────────────────
    // 查询指定位置的物理材质信息（调试用）
    // ─────────────────────────────────────────────
    void DebugQueryPhysMaterial(FVector WorldLocation) const
    {
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.bReturnPhysicalMaterial = true;
        Params.bTraceComplex = true;

        if (!GetWorld()->LineTraceSingleByChannel(
            Hit, WorldLocation + FVector(0,0,100), WorldLocation - FVector(0,0,100),
            ECC_Visibility, Params))
        {
            UE_LOG(LogTemp, Warning, TEXT("No ground found at %s"), *WorldLocation.ToString());
            return;
        }

        UPhysicalMaterial* PM = Hit.PhysMaterial.Get();
        if (!PM) { UE_LOG(LogTemp, Warning, TEXT("No PhysMat")); return; }

        UE_LOG(LogTemp, Log, TEXT("=== Physical Material at %s ==="),
            *WorldLocation.ToString());
        UE_LOG(LogTemp, Log, TEXT("  Name:        %s"), *PM->GetName());
        UE_LOG(LogTemp, Log, TEXT("  Friction:    %.3f"), PM->Friction);
        UE_LOG(LogTemp, Log, TEXT("  Restitution: %.3f"), PM->Restitution);
        UE_LOG(LogTemp, Log, TEXT("  Density:     %.3f g/cm³"), PM->Density);
        UE_LOG(LogTemp, Log, TEXT("  SurfaceType: %d"), (int)PM->SurfaceType.GetValue());
    }
};
