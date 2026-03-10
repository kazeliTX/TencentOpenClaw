# 7.7 破碎事件与回调

> **难度**：⭐⭐⭐⭐☆

## 一、OnChaosBreakEvent

```cpp
// 注册破碎事件
UCLASS()
class AFractureListener : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) UGeometryCollectionComponent* GCComp;

    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        if (GCComp)
            GCComp->OnChaosBreakEvent.AddDynamic(
                this, &AFractureListener::OnBreak);
    }

    // 每次有碎片断开时触发
    UFUNCTION()
    void OnBreak(const FChaosBreakEvent& BreakEvent)
    {
        UE_LOG(LogTemp, Log,
            TEXT("Break at %s, Vel=%.0f, Mass=%.1f"),
            *BreakEvent.Location.ToString(),
            BreakEvent.Velocity.Size(),
            BreakEvent.Mass);
        
        // 在破碎点播放粒子/音效
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), BreakVFX, BreakEvent.Location);
        UGameplayStatics::PlaySoundAtLocation(
            this, BreakSFX, BreakEvent.Location);
        
        // 根据速度选择音效强度
        float ImpactStrength = BreakEvent.Velocity.Size();
        if (ImpactStrength > 1000.f)
            UGameplayStatics::PlaySoundAtLocation(
                this, HeavyBreakSFX, BreakEvent.Location);
    }

    UPROPERTY(EditAnywhere) UParticleSystem* BreakVFX;
    UPROPERTY(EditAnywhere) USoundBase*      BreakSFX;
    UPROPERTY(EditAnywhere) USoundBase*      HeavyBreakSFX;
};
```

## 二、OnChaosRemovalEvent

```cpp
// 碎片被移除（超出 MinMassThreshold，自动销毁）时触发
GCComp->OnChaosRemovalEvent.AddDynamic(
    this, &AFractureListener::OnRemoval);

UFUNCTION()
void OnRemoval(const FChaosRemovalEvent& RemovalEvent)
{
    // 在碎片消失位置生成特效（粉尘/烟雾）
    UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(), DustVFX, RemovalEvent.Location);
}
```

## 三、延伸阅读

- 📄 [7.8 程序化破碎](./08-procedural-fracture.md)
- 📄 [代码示例：破碎触发](./code/01_fracture_trigger.cpp)
