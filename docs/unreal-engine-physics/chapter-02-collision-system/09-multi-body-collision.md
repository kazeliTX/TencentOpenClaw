# 2.9 多碰撞体组合与骨骼碰撞

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、骨骼网格的碰撞结构

```
骨骼网格（Skeletal Mesh）的碰撞不同于 Static Mesh：

Static Mesh：
  碰撞形状 = AggGeom（几个简单体组合）
  整体一个 FBodyInstance

Skeletal Mesh：
  使用 Physics Asset（PhysicsAsset / .phat 文件）
  每块骨骼可以有独立的碰撞体（FBodySetup）
  多个 FBodyInstance → 骨骼物理的基础

骨骼碰撞常见配置（角色）：
  骨骼              碰撞形状    用途
  ─────────────────────────────────────────
  pelvis（骨盆）   胶囊        布娃娃主体
  spine_01（脊柱） 胶囊        上身
  spine_03（上脊）  胶囊        肩膀区域
  head（头部）     球形        头部命中检测
  upperarm_l/r     胶囊        手臂
  lowerarm_l/r     胶囊        前臂
  thigh_l/r        胶囊        大腿
  calf_l/r         胶囊        小腿

注意：
  骨骼碰撞用于 Ragdoll（物理模拟）
  也用于精确的骨骼级别 Hit 检测（识别打到头/手臂）
  详细配置见 第五章 Physics Asset & Ragdoll
```

---

## 二、运行时按骨骼设置碰撞

```cpp
// 只对特定骨骼启用物理（Kinematic → Simulated）
SkeletalMesh->SetAllBodiesBelowSimulatePhysics(
    FName("pelvis"),   // 骨骼名称（该骨骼及其子骨骼）
    true,              // bSimulate
    true               // bIncludeSelf（包含pelvis本身）
);

// 只对头骨设置碰撞响应
FBodyInstance* HeadBI = SkeletalMesh->GetBodyInstance(FName("head"));
if (HeadBI)
{
    HeadBI->SetResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
    // WeaponTrace 打到头 → Block（可以判断爆头）
}

// 对所有骨骼设置碰撞 Profile
SkeletalMesh->SetCollisionProfileName(FName("Ragdoll"));

// 获取被击中的骨骼名称（从 Hit 事件）
void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, ...)
{
    FName HitBone = HitResult.BoneName;
    if (HitBone == "head")
    {
        // 爆头！
        ApplyHeadshotDamage();
    }
}
```

---

## 三、多碰撞体组合 Actor

```
一个 Actor 可以有多个 Component，每个 Component 有独立碰撞：

案例：炸弹箱（有把手）
  BoxComponent（箱体）：
    Profile = PhysicsActor
    Simulate Physics = true
    Mass = 20 kg
  
  CapsuleComponent（把手，可拖拽）：
    Profile = PhysicsActor
    Simulate Physics = true（跟随箱体）
    AttachedTo BoxComponent
  
  SphereComponent（触发引信，不参与物理）：
    Profile = Trigger
    CollisionEnabled = QueryOnly
    Overlap → 检测近距离角色

组合 Actor 的碰撞事件：
  每个 Component 独立触发自己的 Hit/Overlap
  Actor.OnActorHit 汇总所有 Component 的 Hit
  需要判断具体是哪个 Component 被击中：
  
  void OnHit(..., const FHitResult& Hit)
  {
      if (Hit.Component == HandleComp)
          PlayerGrabbedHandle();
      else if (Hit.Component == BodyComp)
          PlayBoxHitSound();
  }
```

---

## 四、延伸阅读

- 📄 [2.10 碰撞过滤](./10-collision-filtering.md)
- 📖 [第五章：Physics Asset & Ragdoll](../chapter-05-physics-asset-ragdoll/)
