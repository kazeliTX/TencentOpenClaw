# 7.9 物理与动画的混合过渡

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、混合的核心挑战

```
动画 ↔ 物理的混合面临两个挑战：

挑战 1：姿势不连续（Pose Discontinuity）
  动画姿势与物理当前姿势差异很大时直接切换
  → 骨骼瞬间跳变（视觉穿帮）

挑战 2：速度不连续（Velocity Discontinuity）
  物理体有速度时切换为动画（速度突然清零）
  → 运动感突然消失
  
解决方案：
  动画 → 物理：用 Rigid Body 节点混合（Alpha 0→1）
  物理 → 动画：用 Pose Snapshot + Blend 混合
```

---

## 二、过渡策略总览

```
不同过渡场景的解决方案：

场景                      方案
────────────────────────────────────────────────────────
站立 → 布娃娃（死亡）      Rigid Body Alpha 渐变 → 完整物理
布娃娃 → 站立（恢复）      Pose Snapshot → Get Up 动画 Blend
局部受击反应              Physical Animation Blend Weight 降低/恢复
次级运动（配件）           AnimDynamics / Spring Bone（始终运行）
死亡倒地                  Rigid Body 过渡 → 最终布娃娃
```

---

## 三、Inertialization 辅助过渡

```
UE5 的 Inertialization 节点（惯性化）可用于物理过渡：

原理：
  Inertialization 记录切换时刻的骨骼位置和速度
  过渡期间通过指数衰减平滑从旧姿势过渡到新姿势
  比普通 Blend 更自然（保留速度连续性）

用于物理过渡：
  [物理恢复后的动画]
        │
  [Inertialization（Request）]
    Blend Time: 0.3s（过渡时间）
        │
  [Output Pose]

触发时机：
  在 BeginRagdollRecovery() 中调用：
  AnimInstance->RequestInertialization(0.3f);
  → 从布娃娃最终姿势平滑过渡到起立动画（考虑速度连续性）
```

---

## 四、PhysicsBlendWeight 动态调整

```cpp
// 精细的 PhysicsBlendWeight 控制（按骨骼链）
void AMyCharacter::SetUpperBodyPhysicsBlend(float BlendWeight)
{
    USkeletalMeshComponent* Mesh = GetMesh();
    if (!Mesh) return;

    // 只调整 spine_01 以上的骨骼（上半身）
    TArray<FName> UpperBodyBones = {
        FName("spine_01"), FName("spine_02"), FName("spine_03"),
        FName("neck_01"), FName("head"),
        FName("clavicle_l"), FName("upperarm_l"), FName("lowerarm_l"), FName("hand_l"),
        FName("clavicle_r"), FName("upperarm_r"), FName("lowerarm_r"), FName("hand_r"),
    };

    for (const FName& Bone : UpperBodyBones)
    {
        FBodyInstance* Body = Mesh->GetBodyInstance(Bone);
        if (Body)
        {
            Body->PhysicsBlendWeight = FMath::Clamp(BlendWeight, 0.f, 1.f);
        }
    }
}
```

---

## 五、延伸阅读

- 🔗 [Inertialization 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/inertialization-in-unreal-engine)
- 🔗 [Physical Animation 混合](https://dev.epicgames.com/documentation/en-us/unreal-engine/physical-animation-in-unreal-engine)
