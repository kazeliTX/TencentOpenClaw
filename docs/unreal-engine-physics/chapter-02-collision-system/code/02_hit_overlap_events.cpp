// =============================================================================
// 02_hit_overlap_events.cpp
// Hit & Overlap 事件完整处理示例：子弹、触发区域、拾取物
// 对应文档：chapter-02-collision-system/05-hit-events.md & 06-overlap-events.md
// =============================================================================
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "HitOverlapDemo.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// 子弹 Actor（Hit 事件示例）
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class ABulletActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) USphereComponent*     CollisionSphere;
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* BulletMesh;
    UPROPERTY(EditAnywhere)    float                 Damage = 25.f;
    UPROPERTY(EditAnywhere)    UParticleSystem*      HitParticle;
    UPROPERTY(EditAnywhere)    USoundBase*           HitSound;

    bool bHasHit = false;   // 防止多次 Hit

    ABulletActor()
    {
        CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
        CollisionSphere->SetSphereRadius(3.f);
        CollisionSphere->SetCollisionProfileName(FName("Projectile"));  // 自定义
        CollisionSphere->SetNotifyRigidBodyCollision(true);  // 开启 Hit 事件
        RootComponent = CollisionSphere;

        BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
        BulletMesh->SetupAttachment(CollisionSphere);
        BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);// Mesh不参与碰撞
    }

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        CollisionSphere->OnComponentHit.AddDynamic(this, &ABulletActor::OnHit);
    }

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
    {
        if (bHasHit) return;  // 只处理第一次 Hit
        bHasHit = true;

        // 1. 造成伤害（根据被击骨骼判断部位）
        float ActualDamage = Damage;
        if (Hit.BoneName == FName("head"))
            ActualDamage *= 2.5f;  // 爆头 2.5 倍伤害
        else if (Hit.BoneName == FName("torso") || Hit.BoneName == FName("spine_01"))
            ActualDamage *= 1.0f;
        else
            ActualDamage *= 0.7f;  // 四肢减伤

        if (OtherActor && OtherActor != this)
            UGameplayStatics::ApplyPointDamage(OtherActor, ActualDamage,
                GetActorForwardVector(), Hit, GetInstigatorController(),
                this, nullptr);

        // 2. 碰撞冲量推开物体
        if (OtherComp && OtherComp->IsSimulatingPhysics())
        {
            FVector ImpulseDir = GetActorForwardVector();
            float   ImpulseMag = 30000.f;
            OtherComp->AddImpulseAtLocation(ImpulseDir * ImpulseMag, Hit.ImpactPoint);
        }

        // 3. 命中特效
        if (HitParticle)
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle,
                Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

        // 4. 命中音效（基于物理材质）
        USoundBase* Sound = HitSound;
        if (Hit.PhysMaterial.IsValid())
        {
            // 根据 SurfaceType 选择不同音效
            // EPhysicalSurface Surface = Hit.PhysMaterial->SurfaceType;
        }
        if (Sound)
            UGameplayStatics::PlaySoundAtLocation(this, Sound, Hit.ImpactPoint);

        // 5. 弹孔贴花（延迟销毁自身）
        SetActorHiddenInGame(true);
        CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        SetLifeSpan(0.1f);  // 0.1s 后自动销毁
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// 触发区域（Overlap 事件示例）
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class ATriggerZoneActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) USphereComponent* ZoneSphere;
    TSet<AActor*> ActorsInsideZone;
    bool bIsActive = true;

    ATriggerZoneActor()
    {
        ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Zone"));
        ZoneSphere->SetSphereRadius(500.f);
        ZoneSphere->SetCollisionProfileName(FName("OverlapAllDynamic"));
        ZoneSphere->SetGenerateOverlapEvents(true);
        RootComponent = ZoneSphere;
    }

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &ATriggerZoneActor::OnBegin);
        ZoneSphere->OnComponentEndOverlap.AddDynamic(this, &ATriggerZoneActor::OnEnd);
        
        // 场景开始时可能已在区域内的对象（不会触发 BeginOverlap）
        TArray<AActor*> AlreadyInside;
        ZoneSphere->GetOverlappingActors(AlreadyInside, ACharacter::StaticClass());
        for (AActor* A : AlreadyInside)
            HandleActorEnter(A);
    }

    UFUNCTION()
    void OnBegin(UPrimitiveComponent* Comp, AActor* Other,
        UPrimitiveComponent* OtherComp, int32 OtherIdx,
        bool bFromSweep, const FHitResult& Sweep)
    {
        if (!bIsActive || !IsValid(Other) || Other == this) return;
        // 防止同一 Actor 的多个 Component 重复触发
        if (ActorsInsideZone.Contains(Other)) return;
        HandleActorEnter(Other);
    }

    UFUNCTION()
    void OnEnd(UPrimitiveComponent* Comp, AActor* Other,
        UPrimitiveComponent* OtherComp, int32 OtherIdx)
    {
        if (!IsValid(Other)) return;
        // 检查该 Actor 是否还有其他 Component 在区域内
        TArray<UPrimitiveComponent*> OtherComps;
        Other->GetComponents<UPrimitiveComponent>(OtherComps);
        bool bStillOverlapping = false;
        for (UPrimitiveComponent* C : OtherComps)
        {
            if (C != OtherComp && ZoneSphere->IsOverlappingComponent(C))
            {
                bStillOverlapping = true;
                break;
            }
        }
        if (!bStillOverlapping)
            HandleActorExit(Other);
    }

    void HandleActorEnter(AActor* Actor)
    {
        ActorsInsideZone.Add(Actor);
        UE_LOG(LogTemp, Log, TEXT("[Zone] %s entered. Count=%d"),
            *Actor->GetName(), ActorsInsideZone.Num());
        // 触发事件（如应用 Buff/Debuff）
    }

    void HandleActorExit(AActor* Actor)
    {
        ActorsInsideZone.Remove(Actor);
        UE_LOG(LogTemp, Log, TEXT("[Zone] %s exited. Count=%d"),
            *Actor->GetName(), ActorsInsideZone.Num());
    }

    // 每帧清理已销毁的 Actor
    virtual void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);
        ActorsInsideZone.RemoveAll([](AActor* A){ return !IsValid(A); });
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// 拾取物（一次性 Overlap 示例）
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class APickupActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere) USphereComponent*     PickupRange;
    UPROPERTY(VisibleAnywhere) UStaticMeshComponent* ItemMesh;
    bool bPickedUp = false;

    APickupActor()
    {
        PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("Range"));
        PickupRange->SetSphereRadius(80.f);
        PickupRange->SetCollisionProfileName(FName("OverlapAllDynamic"));
        PickupRange->SetGenerateOverlapEvents(true);
        RootComponent = PickupRange;

        ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
        ItemMesh->SetupAttachment(PickupRange);
        ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        PickupRange->OnComponentBeginOverlap.AddDynamic(this, &APickupActor::OnPickup);
    }

    UFUNCTION()
    void OnPickup(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*,
        int32, bool, const FHitResult&)
    {
        if (bPickedUp) return;
        if (!Other || !Other->IsA(ACharacter::StaticClass())) return;
        
        bPickedUp = true;
        // 立即关闭 Overlap，避免再次触发
        PickupRange->SetGenerateOverlapEvents(false);
        
        UE_LOG(LogTemp, Log, TEXT("%s picked up item!"), *Other->GetName());
        // 执行拾取逻辑后销毁
        Destroy();
    }
};
