# 3.3 表面类型（SurfaceType）与音效/粒子

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、SurfaceType 系统

```
SurfaceType 是物理材质上的枚举标签：
  用于区分不同表面（石头/木头/金属/泥土...）
  供游戏逻辑查询（脚步音效/弹孔贴花/粒子特效）
  与物理计算无关（纯逻辑标签）

配置位置：
  Project Settings → Physics → Physical Surface（物理表面）
  可添加最多 62 个自定义表面类型（SurfaceType1~SurfaceType62）
  
推荐命名规范：
  SurfaceType1  = Stone（石头/混凝土）
  SurfaceType2  = Wood（木材）
  SurfaceType3  = Metal（金属）
  SurfaceType4  = Dirt（泥土/土地）
  SurfaceType5  = Grass（草地）
  SurfaceType6  = Sand（沙地/沙漠）
  SurfaceType7  = Ice（冰面）
  SurfaceType8  = Water（水面/浅水）
  SurfaceType9  = Glass（玻璃）
  SurfaceType10 = Flesh（肉体/角色）
  SurfaceType11 = Rubber（橡胶）
  SurfaceType12 = Fabric（布料/地毯）

C++ 枚举访问：
  EPhysicalSurface::SurfaceType1
  EPhysicalSurface::SurfaceType_Default（默认/未指定）
```

---

## 二、脚步音效系统（Footstep Sound）

```
完整脚步音效流程：

1. 为每种地面创建 Physical Material
   PM_Stone: SurfaceType = SurfaceType1 (Stone)
   PM_Wood:  SurfaceType = SurfaceType2 (Wood)
   ...

2. 在骨骼动画的 AnimNotify 中触发脚步检测
   AnimNotify_Footstep → 向脚底发射射线

3. 射线命中获取 PhysicalMaterial，读取 SurfaceType

4. 根据 SurfaceType 查表选择音效

5. 播放音效（带随机变体防止重复感）

骨骼脚步检测代码：
void AMyCharacter::OnFootstep(bool bLeftFoot)
{
    // 脚骨骼坐标
    FName FootBone = bLeftFoot ? FName("foot_l") : FName("foot_r");
    FVector FootLoc = GetMesh()->GetBoneLocation(FootBone);
    
    // 向下射线
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.bReturnPhysicalMaterial = true;
    Params.bTraceComplex = false;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, FootLoc + FVector(0,0,10), FootLoc - FVector(0,0,50),
        ECC_Visibility, Params);
    
    if (!bHit) return;
    
    // 获取表面类型
    EPhysicalSurface Surface = UPhysicalMaterial::DetermineSurfaceType(
        Hit.PhysMaterial.Get());
    
    // 查表选音效
    PlayFootstepSound(Surface, FootLoc);
    
    // 生成脚步粒子（泥地/雪地留下足迹）
    if (Surface == SurfaceType4) // Dirt
        SpawnFootprintDecal(FootLoc, GetActorRotation());
}

void AMyCharacter::PlayFootstepSound(EPhysicalSurface Surface, FVector Location)
{
    USoundBase* Sound = FootstepSounds.FindRef(Surface);
    if (!Sound)
        Sound = FootstepSounds.FindRef(SurfaceType_Default);
    if (!Sound) return;
    
    // 随机音调防止重复感
    float Pitch = FMath::RandRange(0.9f, 1.1f);
    UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, 1.f, Pitch);
}

// TMap 存储音效
UPROPERTY(EditAnywhere, Category="Footstep")
TMap<TEnumAsByte<EPhysicalSurface>, USoundBase*> FootstepSounds;
```

---

## 三、弹孔贴花系统（Bullet Decal）

```
不同表面产生不同弹孔贴花：

void ABullet::SpawnImpactEffect(const FHitResult& Hit)
{
    EPhysicalSurface Surface = SurfaceType_Default;
    if (Hit.PhysMaterial.IsValid())
        Surface = Hit.PhysMaterial->SurfaceType;
    
    // 查找对应贴花
    UMaterialInterface* DecalMat = ImpactDecals.FindRef(Surface);
    if (!DecalMat) DecalMat = ImpactDecals.FindRef(SurfaceType_Default);
    
    // 粒子特效
    UParticleSystem* Particle = ImpactParticles.FindRef(Surface);
    
    if (DecalMat)
    {
        // 生成弹孔贴花
        UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
            GetWorld(), DecalMat,
            FVector(5.f, 5.f, 0.1f),  // 贴花尺寸
            Hit.ImpactPoint,
            Hit.ImpactNormal.Rotation(),
            10.f);  // 10秒后自动消失
    }
    
    if (Particle)
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), Particle, Hit.ImpactPoint,
            Hit.ImpactNormal.Rotation());
    
    // 穿透效果（如玻璃碎裂）
    if (Surface == SurfaceType9) // Glass
        TriggerGlassBreak(Hit);
}
```

---

## 四、延伸阅读

- 📄 [3.4 物理约束总览](./04-constraint-overview.md)
- 🔗 [Physical Surface Types](https://dev.epicgames.com/documentation/en-us/unreal-engine/physical-materials-in-unreal-engine)
