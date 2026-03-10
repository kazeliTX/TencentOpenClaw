# 2.10 碰撞过滤与忽略

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、Actor 级别忽略

```cpp
// 忽略指定 Actor（双向）
MyComp->IgnoreActorWhenMoving(OtherActor, true);
// 效果：移动扫掠时忽略 OtherActor（不产生碰撞阻挡）
// 注意：不影响 Overlap 事件！只影响 Movement 的碰撞

// 查询时忽略（用于 LineTrace）
FCollisionQueryParams Params;
Params.AddIgnoredActor(OtherActor);
Params.AddIgnoredActors(TArray<AActor*>{A1, A2, A3});

// 关闭整个 Actor 的碰撞
SetActorEnableCollision(false);  // 所有 Component 全部禁用

// 临时忽略（如无敌状态）
void StartInvincible()
{
    Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
    // WeaponTrace 无法检测到该 Actor
}
void EndInvincible()
{
    Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
}
```

---

## 二、Component 级别过滤

```cpp
// 只禁用特定 Component 的碰撞
WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

// 动态切换（收起武器时禁用武器碰撞）
void SheatheWeapon()
{
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void DrawWeapon()
{
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

// 按通道过滤（只对某类对象忽略）
// 角色穿过门帘（布料）时忽略布料碰撞
ClothComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

// 团队友军不碰撞
// 同队角色设置 Ignore 彼此
void SetTeamCollision(ACharacter* OtherChar, bool bIgnore)
{
    MyMesh->IgnoreActorWhenMoving(OtherChar, bIgnore);
    OtherChar->GetMesh()->IgnoreActorWhenMoving(this, bIgnore);
}
```

---

## 三、UCollisionIgnoreComponent（UE5）

```
UE5 引入 UCollisionIgnoreComponent：
  更高效的批量忽略机制
  支持按 Tag/Class 批量忽略
  
  用法：
    添加到 Actor：
      UCollisionIgnoreComponent* IgnoreComp =
          AddComponent<UCollisionIgnoreComponent>();
    
    忽略特定 Actor：
      IgnoreComp->IgnoreActor(OtherActor);
    
    忽略某类对象：
      IgnoreComp->IgnoreClass = AProjectile::StaticClass();
    
    忽略 Tag：
      IgnoreComp->IgnoreTags.Add(FName("TeamA"));

  应用场景：
    子弹不打发射者（忽略自身）
    团队伤害关闭（忽略同队 Tag）
    Boss 的特殊攻击忽略特定目标
```

---

## 四、延伸阅读

- 📄 [代码示例：碰撞设置](./code/01_collision_setup.cpp)
- 🔗 [Collision Filtering](https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-in-unreal-engine)
