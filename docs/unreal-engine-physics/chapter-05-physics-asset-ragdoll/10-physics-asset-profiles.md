# 5.10 Physics Asset Profile 管理

> **难度**：⭐⭐⭐⭐☆

## 一、Profile 类型

```
Physics Asset 支持两种 Profile：

1. Physical Animation Profile（物理动画 Profile）
   控制 Physical Animation Component 的参数
   决定"物理如何跟随动画"
   参数：StrengthMultiplier, bIsLocalSimulation, etc.

2. Constraint Profile（约束 Profile）
   控制每个关节的限制参数
   不同场景切换不同约束配置
   
   常见 Constraint Profile：
     Default     ：完整布娃娃关节限制
     Kinematic   ：所有关节锁定（完全动画驱动）
     HitReaction ：松动一点，允许命中反应摆动
     Underwater  ：更大的摆动范围（水中漂浮感）
```

## 二、在 PhAT 中创建 Profile

```
1. 打开 PhAT
2. 上方下拉菜单 → Constraint Profiles → New Profile
3. 命名（如 "Ragdoll", "Kinematic", "HitReaction"）
4. 在 Skeleton Tree 中全选所有约束
5. 设置约束参数
6. Profile 下拉 → "Assign to Selected Constraints"
7. 保存

注意：Default Profile 永远存在，无法删除
```

## 三、C++ 切换 Profile

```cpp
// 切换所有约束到指定 Profile
GetMesh()->SetConstraintProfileForAll(FName("Ragdoll"), true);
//                                                       ↑ bDefaultIfNotFound
//   true：如果某个约束没有对应 Profile，使用 Default
//   false：该约束不变

// 切换单个骨骼的约束 Profile
GetMesh()->SetConstraintProfile(
    FName("spine_01"),  // 骨骼名
    FName("HitReaction"), // Profile 名
    true);

// 常见使用序列：
// 正常行走：SetConstraintProfileForAll("Kinematic")
// 被击中：  SetConstraintProfile("upperarm_r", "HitReaction")
// 死亡：    SetConstraintProfileForAll("Ragdoll")
//           + SetSimulatePhysics(true)
// 起身：    SetConstraintProfileForAll("Kinematic")
//           + SetSimulatePhysics(false)
```

## 四、Physical Animation Profile

```cpp
// UPhysicalAnimationComponent 使用
UPROPERTY(VisibleAnywhere) UPhysicalAnimationComponent* PhysAnim;

void BeginPlay()
{
    PhysAnim->SetSkeletalMeshComponent(GetMesh());
    
    // 应用 Physical Animation Profile（受击时身体追随动画但仍有物理感）
    PhysAnim->ApplyPhysicalAnimationProfileBelow(
        FName("spine_01"),  // 从哪根骨骼开始
        FName("HitReaction"),  // Profile 名
        true,   // bIncludeSelf
        true);  // bClearNotFound（清除其余骨骼的物理动画）
    
    // 必须开启物理模拟才能生效
    GetMesh()->SetAllBodiesBelowSimulatePhysics(
        FName("spine_01"), true, true);
}
```

## 五、延伸阅读

- 📄 [代码示例：完整 Ragdoll 系统](./code/01_ragdoll_system.cpp)
- 🔗 [Physics Asset Profiles](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-asset-editor-in-unreal-engine)
