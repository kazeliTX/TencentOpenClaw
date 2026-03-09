# 5.7 Full Body IK（FBIK）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、FBIK 的特点

```
传统 IK 的局限：
  Two-Bone IK：只解两根骨骼
  FABRIK：只解一条链，不同链之间无联动

Full Body IK（FBIK）：
  同时对多个骨骼链（脊椎 + 四肢）进行联动求解
  给定多个效应器目标 → 求解整个骨骼树的最优姿势
  保持关节约束和骨骼刚性

典型 FBIK 用途：
  角色同时跪下 + 双手触地（多约束同时满足）
  爬行动物（四肢协调）
  角色与环境精确交互（双手扶墙 + 脚踩台阶）
  VR 角色（头显 + 双手控制器 → 驱动全身）
```

---

## 二、在 AnimBP 中使用 FBIK 节点

```
AnimGraph 节点：Full Body IK
（需要先配置好 IK Rig，见 5.6 章节）

节点参数概览：
  IK Rig Asset: IKR_Mannequin

Effector Goals（每个效应器的目标）：
  foot_l_goal:
    Target Location: LeftFootIKTarget    ← AnimInstance 变量
    Target Rotation: LeftFootRotation
    Position Alpha:  1.0
    Rotation Alpha:  0.5（脚旋转权重可以降低些）
  
  foot_r_goal: （同上，右脚）
  
  hand_l_goal:
    Target Location: LeftHandIKTarget
    Position Alpha: LeftHandIKAlpha     ← 根据武器动态控制
  
  head_goal:
    Target Location: LookAtTargetCS
    Position Alpha: 0.0                 （不移动头部位置）
    Rotation Alpha: 0.8                 （只控制头部旋转）

Root Behavior:
  Pinned（骨盆固定，IK 向四肢传播）
  Free（骨盆可以移动，更自然但计算量大）
```

---

## 三、FBIK 与传统 IK 混合

```
实践建议：不要用 FBIK 替代所有 IK！

混合策略：
  脚步 IK（常见，低性能）：用 Two-Bone IK（最快）
  头部注视（常见）：用 Look At 节点（极快）
  双手持枪（常见）：用 Two-Bone IK（快）
  复杂交互姿势（偶发）：用 FBIK（较慢，按需开启）

FBIK 只在以下情况开启：
  角色进入"精确交互"状态
  多个效应器同时需要精确对齐时
  Alpha 从 0 渐变到 1，交互结束时渐变回 0

性能参考（100 个角色）：
  全身 FBIK 每角色：~0.5-1.5ms
  只用基础 IK 节点：~0.1-0.3ms
  → FBIK 开销是传统 IK 的 3-5 倍
```

---

## 四、VR 全身 IK 示例

```cpp
// VR 场景：头显 + 双手控制器 → 全身 IK
void UVRAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    // 获取 VR 追踪数据
    FVector  HMDPosition; FRotator HMDRotation;
    UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRotation, HMDPosition);

    // 转换到 Character Component Space
    FTransform WorldToComp = CachedMesh->GetComponentTransform().Inverse();

    HeadIKTarget      = WorldToComp.TransformPosition(HMDPosition);
    HeadIKRotation    = HMDRotation;

    // 左右手控制器
    LeftHandIKTarget  = WorldToComp.TransformPosition(
        LeftMotionController->GetComponentLocation());
    RightHandIKTarget = WorldToComp.TransformPosition(
        RightMotionController->GetComponentLocation());
    LeftHandIKRotation  = LeftMotionController->GetComponentRotation();
    RightHandIKRotation = RightMotionController->GetComponentRotation();
}
// AnimGraph: FBIK 节点读取上面的变量
// → 根据头 + 双手位置求解全身姿势
```

---

## 五、延伸阅读

- 🔗 [Full Body IK 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/full-body-ik-in-unreal-engine)
- 🔗 [IK Rig 配置指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-ik-rig)
