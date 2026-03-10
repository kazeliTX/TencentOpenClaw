# 7.4 Field System（力场/应变场）

> **难度**：⭐⭐⭐⭐⭐

## 一、Field System 概念

```
Field System：可编程的物理场
  向特定空间区域内的 Chaos 粒子施加力/应变/速度等影响
  
  与 RadialForce 区别：
    RadialForce：简单径向冲量，只影响刚体速度
    Field System：更强大，可以直接触发破碎（内力场）
    
Field 类型：
  ExternalStrain（外部应变场）：✅ 触发破碎最常用
    向 GC 施加应变（Strain），超过 DamageThreshold 的部分破碎
    
  LinearForce（线性力场）：向碎片施加力
  RadialVector（径向向量场）：从中心向外的向量
  UniformVector（均匀向量场）：恒定方向的向量
  
Field 形状（Falloff）：
  Sphere：球形衰减（最常用，爆炸感）
  Box：盒形无衰减
  Plane：半空间（平面一侧生效）
  
Field 衰减（Falloff Type）：
  None：范围内均匀
  Linear：线性衰减
  Inverse：平方反比
  Squared：平方衰减
```

## 二、通过 AnchorField 控制固定/激活

```
AnchorField（锚定场）：控制哪些碎片固定不动

AnchorField 类型：
  Static → 使范围内的碎片保持静止（固定）
  Dynamic → 使范围内的碎片激活（运动）
  Sleeping → 使碎片入睡（暂停模拟）
  
典型应用：
  建筑倒塌时底部保持固定，只有被打击的部分掉落
  
  SetupAnchorField:
    范围覆盖地面部分 → State = Static（地基不移动）
    玩家打击范围 → State = Dynamic（被打中部分激活）
```

## 三、运行时 Field 施加（C++ API）

```cpp
// 在爆炸位置施加应变场，触发周围 GC 破碎
void TriggerFractureWithField(UWorld* World, FVector Center, float Radius, float Strain)
{
    // 创建径向力场 Actor
    AFieldSystemActor* FieldActor = World->SpawnActor<AFieldSystemActor>(
        AFieldSystemActor::StaticClass(), Center, FRotator::ZeroRotator);
    
    if (!FieldActor) return;
    
    UFieldSystemComponent* FSC = FieldActor->GetFieldSystemComponent();
    
    // 构建径向应变场：
    //   RadialFalloff（从中心到 Radius 线性衰减）× Magnitude
    //   → 超过 GC 的 DamageThreshold 时触发破碎
    FSC->ApplyLinearForce(false,
        EFieldPhysicsType::Field_ExternalClusterStrain,
        FFieldSystemMetaDataFilter(),
        Center,
        Strain,           // 应变大小
        Radius,           // 影响半径
        EFieldFalloffType::Field_FallOff_Linear);
    
    // 5帧后销毁 Field Actor（单次爆炸）
    FieldActor->SetLifeSpan(0.1f);
}
```

## 四、延伸阅读

- 📄 [7.5 破碎阈值](./05-break-threshold.md)
- 📄 [7.6 Anchor Fields](./06-anchor-fields.md)
- 📄 [代码示例：Field System](./code/02_field_system.cpp)
