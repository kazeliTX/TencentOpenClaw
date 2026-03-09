# 7.3 Physical Animation Component

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、Physical Animation Component 的作用

```
传统动画：骨骼完全由动画驱动（FK/IK），物理只用于布娃娃
Physical Animation：骨骼同时受动画和物理约束驱动

效果：
  动画姿势作为"目标姿势"
  物理约束将骨骼从当前物理状态"拉向"目标姿势
  强度可调（0=纯物理，1=纯动画）
  
应用场景：
  ✅ 受击反应（上半身被打时物理晃动，下半身动画继续）
  ✅ 次级运动（手臂/武器在快速移动时的惯性效果）
  ✅ 环境响应（角色被风/水流推动时的自然晃动）
  ✅ 程序化细节（头发/披风的物理辅助运动）
```

---

## 二、添加 Physical Animation Component

```cpp
// 在角色头文件中
UPROPERTY(VisibleAnywhere, Category="Physics")
UPhysicalAnimationComponent* PhysicalAnimation;

// 构造函数
AMyCharacter::AMyCharacter()
{
    PhysicalAnimation = CreateDefaultSubobject<UPhysicalAnimationComponent>(
        TEXT("PhysicalAnimation"));
}

// BeginPlay 中配置
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // 关联到 Skeletal Mesh
    PhysicalAnimation->SetSkeletalMeshComponent(GetMesh());
    
    // 应用预设的 Physical Animation Profile（在 Physics Asset 中配置）
    PhysicalAnimation->ApplyPhysicsProfile(GetMesh(), FName("Default"));
    
    // 对 spine_01 及以上骨骼开启物理模拟（上半身物理）
    GetMesh()->SetAllBodiesBelowSimulatePhysics(
        FName("spine_01"), true, true);
}
```

---

## 三、Physical Animation Profile 配置

```
在 Physics Asset（PhAT）中创建 Physical Animation Profile：

1. PhAT 编辑器 → Profiles 面板 → New Profile → "HitReaction"

2. 为需要物理的骨骼添加 Physical Animation Data：
   选中 Body（如 spine_02）→ 右键 → Add to Profile

3. 每个骨骼的 Physical Animation Data 参数：
   Is Local Simulation: true
     （在骨骼本地空间模拟，更稳定）
   
   Orientation Strength: 1000.0
     （将骨骼拉回目标旋转的力度）
   
   Angular Velocity Strength: 100.0
     （角速度阻尼，防止振荡）
   
   Position Strength: 1000.0
     （将骨骼拉回目标位置的力度，通常比旋转稍低）
   
   Velocity Strength: 100.0
     （线速度阻尼）
   
   Max Linear Force: 10000.0
     （最大线性力限制，防止过大力导致不稳定）
   
   Max Angular Force: 10000.0
     （最大旋转力限制）

典型参数：
  受击反应（短暂物理）：Orientation Strength 500, Angular Velocity 50
  次级运动（持续物理）：Orientation Strength 2000, Angular Velocity 200
```

---

## 四、运行时调整物理强度

```cpp
// 动态调整物理动画强度（0=纯物理，1=完全跟随动画目标）
void AMyCharacter::SetPhysicsBlendWeight(float Weight)
{
    // Weight: 0.0 = 完全物理（忽略动画目标）
    //         1.0 = 完全跟随动画（物理约束最强）
    GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(
        FName("spine_01"), Weight, false, true);
}

// 受击时降低物理权重，然后逐渐恢复
void AMyCharacter::OnHit(FVector HitLocation, float Strength)
{
    // 受击时物理权重降到低值（让骨骼自由晃动）
    SetPhysicsBlendWeight(0.3f);
    
    // 在受击点施加冲量（使受击骨骼弹出）
    FName HitBone = GetMesh()->FindClosestBone(HitLocation);
    GetMesh()->AddImpulseAtLocation(
        (HitLocation - GetActorLocation()).GetSafeNormal() * Strength * 100.f,
        HitLocation);
    
    // 0.3 秒后恢复物理权重
    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer, [this]()
    {
        SetPhysicsBlendWeight(1.0f); // 恢复跟随动画
    }, 0.3f, false);
}
```

---

## 五、延伸阅读

- 📄 [7.4 受击反应系统](./04-hit-reaction.md)
- 📄 [代码示例：Physical Animation 配置](./code/03_physical_anim_setup.cpp)
- 🔗 [Physical Animation Component 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/physical-animation-in-unreal-engine)
