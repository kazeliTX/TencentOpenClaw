# 8.7 逐骨骼物理控制

> **难度**：⭐⭐⭐⭐☆

## 核心内容

```cpp
// 运行时精确控制每根骨骼的物理动画参数
void ConfigurePerBonePhysics(UPhysicalAnimationComponent* PhysAnim)
{
    FPhysicalAnimationData SpineData;
    SpineData.bIsLocalSimulation      = true;
    SpineData.OrientationStrength     = 1000.f;
    SpineData.AngularVelocityStrength = 100.f;
    SpineData.MaxAngularForce         = 50000.f;

    FPhysicalAnimationData HeadData;
    HeadData.bIsLocalSimulation      = true;
    HeadData.OrientationStrength     = 3000.f; // 头部追随更紧密
    HeadData.AngularVelocityStrength = 300.f;
    HeadData.MaxAngularForce         = 100000.f;

    FPhysicalAnimationData ArmData;
    ArmData.bIsLocalSimulation      = true;
    ArmData.OrientationStrength     = 500.f;  // 手臂较软
    ArmData.AngularVelocityStrength = 50.f;

    PhysAnim->ApplyPhysicalAnimationSettings(FName("spine_01"), SpineData);
    PhysAnim->ApplyPhysicalAnimationSettings(FName("head"),     HeadData);
    PhysAnim->ApplyPhysicalAnimationSettings(FName("upperarm_l"), ArmData);
    PhysAnim->ApplyPhysicalAnimationSettings(FName("upperarm_r"), ArmData);
}
```
