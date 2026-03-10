# 5.7 受击反应（部分布娃娃）

> **难度**：⭐⭐⭐⭐⭐

## 一、命中部位检测

```
Hit Event → 根据命中骨骼决定反应强度：

// 骨骼到受击强度映射
TMap<FName, float> HitImpactMap = {
    { "head",      5.0f },  // 爆头 = 最强反应
    { "neck_01",   4.0f },
    { "spine_03",  3.0f },
    { "spine_02",  2.5f },
    { "spine_01",  2.0f },
    { "pelvis",    2.0f },
    { "upperarm_l",1.5f },
    { "upperarm_r",1.5f },
    { "thigh_l",   1.5f },
    { "thigh_r",   1.5f },
    { "lowerarm_l",1.0f },
    { "lowerarm_r",1.0f },
    { "calf_l",    1.0f },
    { "calf_r",    1.0f },
};

void OnBulletHit(const FHitResult& Hit, float BulletImpulse)
{
    FName BoneName = Hit.BoneName;
    float* Multiplier = HitImpactMap.Find(BoneName);
    float Impact = BulletImpulse * (Multiplier ? *Multiplier : 1.0f);
    
    // 施加子弹冲量
    GetMesh()->AddImpulseAtLocation(
        Hit.ImpactNormal * -Impact,  // 子弹方向
        Hit.ImpactPoint,
        BoneName);
}
```

## 二、部分布娃娃（上半身物理 + 下半身动画）

```cpp
void ActivateUpperBodyPhysics(float Duration)
{
    // 只对上半身骨骼开启物理
    TArray<FName> UpperBodyBones = {
        "spine_01", "spine_02", "spine_03",
        "clavicle_l", "clavicle_r",
        "upperarm_l", "upperarm_r",
        "lowerarm_l", "lowerarm_r",
        "hand_l", "hand_r",
        "neck_01", "head"
    };
    
    for (const FName& Bone : UpperBodyBones)
    {
        // 开启该骨骼 Body 的物理模拟
        GetMesh()->SetAllBodiesBelowSimulatePhysics(
            Bone, true, true);
    }
    
    // N 秒后恢复
    FTimerHandle HitTimer;
    GetWorld()->GetTimerManager().SetTimer(HitTimer,
        FTimerDelegate::CreateLambda([this]()
        {
            GetMesh()->SetAllBodiesSimulatePhysics(false);
            GetMesh()->PhysicsBlendWeight = 0.f;
        }), Duration, false);
}
```

## 三、逐渐恢复动画

```
受击反应后的恢复曲线：
  t=0.0：物理权重 = 1.0（完全物理）
  t=0.5：物理权重 = 0.7
  t=1.0：物理权重 = 0.3
  t=1.5：物理权重 = 0.0（完全动画）
  
  推荐使用 UCurveFloat（在资产中设计曲线）
  
  void Tick(float Dt)
  {
      if (bHitReacting && HitRecoverCurve)
      {
          HitReactTime += Dt;
          float Weight = HitRecoverCurve->GetFloatValue(HitReactTime);
          GetMesh()->PhysicsBlendWeight = Weight;
          if (HitReactTime >= HitRecoverCurve->FloatCurve.GetLastKey().Time)
          {
              bHitReacting = false;
              GetMesh()->PhysicsBlendWeight = 0.f;
              GetMesh()->SetAllBodiesSimulatePhysics(false);
          }
      }
  }
```

## 四、延伸阅读

- 📄 [5.8 死亡动画过渡](./08-death-animation.md)
- 📄 [代码示例：部分布娃娃](./code/02_partial_ragdoll.cpp)
