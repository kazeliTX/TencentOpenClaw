# 5.4 脚步 IK 完整实现（地形适配）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 45 分钟
> 这是最常见、也最需要工程经验的 IK 实现

---

## 一、脚步 IK 解决的问题

```
问题场景：
  角色在不平坦地形上行走时，动画假设地面是平的
  → 脚部穿地（踩进斜坡）或悬空（站在台阶边上只有一只脚着地）

脚步 IK 解决方案：
  1. 向下射线检测找到每只脚下方的真实地面
  2. 用 Two-Bone IK 将脚骨骼移到地面位置
  3. 旋转脚骨骼适应地面法线（脚贴地面倾斜）
  4. 移动骨盆高度确保两脚都能着地
  
视觉效果：
  ✓ 脚始终贴地
  ✓ 脚的旋转跟随地面倾斜
  ✓ 上坡/下坡身体高度自动调整
  ✓ 站在台阶边时两脚高度不同
```

---

## 二、完整实现架构

```
脚步 IK 分三层：

Layer 1（C++ AnimInstance）：射线检测，计算目标位置
  NativeUpdateAnimation:
    ├── TraceFootGround(LeftFoot)  → LeftFootIKData
    └── TraceFootGround(RightFoot) → RightFootIKData

Layer 2（AnimGraph）：应用 IK
  ├── Pelvis 偏移（Move骨骼，调整骨盆高度）
  ├── Left Two-Bone IK（腿部）
  ├── Right Two-Bone IK（腿部）
  └── Foot Rotation（脚部贴地旋转）

Layer 3（平滑）：插值过滤，防止抖动
  所有 IK 目标值都做平滑插值（FInterpTo）
```

---

## 三、射线检测实现

```cpp
// AnimInstance 中的脚步 IK 数据
struct FFootIKData
{
    FVector  TargetLocation    = FVector::ZeroVector;  // IK 目标位置（Component Space）
    FRotator TargetRotation    = FRotator::ZeroRotator;// 脚的旋转（贴地）
    float    HitNormalZ        = 1.0f;                 // 地面法线 Z（判断坡度）
    bool     bGroundFound      = false;                // 是否找到地面
};

// AnimInstance 变量（BlueprintReadOnly，供 AnimGraph 读取）
UPROPERTY(BlueprintReadOnly, Category="FootIK")
FVector LeftFootIKTarget   = FVector::ZeroVector;
UPROPERTY(BlueprintReadOnly, Category="FootIK")
FVector RightFootIKTarget  = FVector::ZeroVector;
UPROPERTY(BlueprintReadOnly, Category="FootIK")
FRotator LeftFootRotation  = FRotator::ZeroRotator;
UPROPERTY(BlueprintReadOnly, Category="FootIK")
FRotator RightFootRotation = FRotator::ZeroRotator;
UPROPERTY(BlueprintReadOnly, Category="FootIK")
float PelvisOffset         = 0.0f;  // 骨盆 Z 偏移
UPROPERTY(BlueprintReadOnly, Category="FootIK")
float IKAlpha              = 1.0f;  // 全局 IK 开关（运动时可以渐隐）

// 中间状态（平滑插值用）
FVector  CurrentLeftTarget   = FVector::ZeroVector;
FVector  CurrentRightTarget  = FVector::ZeroVector;
FRotator CurrentLeftRot      = FRotator::ZeroRotator;
FRotator CurrentRightRot     = FRotator::ZeroRotator;
float    CurrentPelvisOffset = 0.0f;

void UMyAnimInstance::UpdateFootIK(float DeltaTime)
{
    if (!CachedChar || !CachedMesh) return;

    // 运动时减弱 IK（防止脚步滑动）
    float TargetAlpha = (GroundSpeed < 10.0f) ? 1.0f :
                        (GroundSpeed < 200.0f) ? FMath::Lerp(1.0f, 0.5f, GroundSpeed / 200.0f) :
                        0.5f;
    IKAlpha = FMath::FInterpTo(IKAlpha, TargetAlpha, DeltaTime, 5.0f);

    // 检测左右脚
    FFootIKData LeftData  = TraceFootGround(FName("foot_l"), DeltaTime);
    FFootIKData RightData = TraceFootGround(FName("foot_r"), DeltaTime);

    // 计算骨盆偏移（取两脚中较低的那个）
    float LeftDelta  = LeftData.bGroundFound  ? LeftData.TargetLocation.Z  : 0.0f;
    float RightDelta = RightData.bGroundFound ? RightData.TargetLocation.Z : 0.0f;
    float TargetPelvisOffset = FMath::Min(LeftDelta, RightDelta);
    TargetPelvisOffset = FMath::Clamp(TargetPelvisOffset, -15.0f, 0.0f); // 只下降不上升

    // 平滑骨盆偏移
    CurrentPelvisOffset = FMath::FInterpTo(
        CurrentPelvisOffset, TargetPelvisOffset, DeltaTime, 8.0f);
    PelvisOffset = CurrentPelvisOffset;

    // 平滑 IK 目标（Component Space）
    if (LeftData.bGroundFound)
    {
        CurrentLeftTarget = FMath::VInterpTo(
            CurrentLeftTarget, LeftData.TargetLocation, DeltaTime, 15.0f);
        CurrentLeftRot = FMath::RInterpTo(
            CurrentLeftRot, LeftData.TargetRotation, DeltaTime, 15.0f);
    }
    if (RightData.bGroundFound)
    {
        CurrentRightTarget = FMath::VInterpTo(
            CurrentRightTarget, RightData.TargetLocation, DeltaTime, 15.0f);
        CurrentRightRot = FMath::RInterpTo(
            CurrentRightRot, RightData.TargetRotation, DeltaTime, 15.0f);
    }

    LeftFootIKTarget   = CurrentLeftTarget;
    RightFootIKTarget  = CurrentRightTarget;
    LeftFootRotation   = CurrentLeftRot;
    RightFootRotation  = CurrentRightRot;
}

FFootIKData UMyAnimInstance::TraceFootGround(FName FootBone, float DeltaTime)
{
    FFootIKData Result;
    if (!CachedMesh) return Result;

    // 获取脚骨骼的世界位置
    FVector FootWorldPos = CachedMesh->GetBoneLocation(FootBone, EBoneSpaces::WorldSpace);

    // 射线起点：脚上方 TraceUpOffset
    const float TraceUpOffset   = 50.0f;
    const float TraceDownOffset = 75.0f;
    FVector TraceStart = FootWorldPos + FVector(0, 0, TraceUpOffset);
    FVector TraceEnd   = FootWorldPos - FVector(0, 0, TraceDownOffset);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(CachedChar);
    Params.bReturnPhysicalMaterial = false;

    bool bHit = CachedMesh->GetWorld()->LineTraceSingleByChannel(
        HitResult, TraceStart, TraceEnd, ECC_Visibility, Params);

    if (!bHit) return Result;

    Result.bGroundFound = true;
    Result.HitNormalZ   = HitResult.ImpactNormal.Z;

    // 计算脚部 IK 目标（Component Space）
    FVector WorldIKTarget = HitResult.ImpactPoint;
    WorldIKTarget.Z += 5.0f; // 轻微抬高，防止穿地
    Result.TargetLocation = CachedMesh->GetComponentTransform()
        .InverseTransformPosition(WorldIKTarget);

    // 从地面法线计算脚部旋转
    FVector Normal = HitResult.ImpactNormal;
    FVector Forward = CachedChar->GetActorForwardVector();
    // 投影到地面切面
    Forward = (Forward - Normal * FVector::DotProduct(Forward, Normal)).GetSafeNormal();
    FVector Right = FVector::CrossProduct(Normal, Forward);
    FMatrix RotMatrix(Forward, Right, Normal, FVector::ZeroVector);
    Result.TargetRotation = RotMatrix.Rotator();

    return Result;
}
```

---

## 四、AnimGraph 配置

```
完整脚步 IK AnimGraph 结构：

[State Machine（基础动画）]
        │
[Pelvis Offset]
  Transform (Modify) Bone: pelvis
  Translation Mode: Add to Existing
  Translation: (0, 0, PelvisOffset)    ← 骨盆下降
  Translation Space: Component Space
        │
[Left Leg Two-Bone IK]
  IK Bone: foot_l
  Effector Location: LeftFootIKTarget （AnimInstance 变量）
  Effector Location Space: BoneSpace（Component Space 的骨骼子空间）
  Joint Target: LeftKneeTarget        （膝盖 Pole Target）
  Alpha: IKAlpha                       ← 全局 IK 权重
        │
[Right Leg Two-Bone IK]
  IK Bone: foot_r
  Effector: RightFootIKTarget
  Alpha: IKAlpha
        │
[Left Foot Rotation]
  Transform (Modify) Bone: foot_l
  Rotation Mode: Replace Existing
  Rotation: LeftFootRotation           ← 贴地旋转
  Rotation Space: World Space
        │
[Right Foot Rotation]
  ... 同左脚
        │
[Output Pose]
```

---

## 五、常见问题

| 问题 | 解决方案 |
|------|---------|
| 脚部穿地 | 增大 IK 目标 Z 偏移（+5~10cm）|
| 脚在台阶边缘抖动 | 对 IK 目标增大平滑速度（FInterpTo Speed=15~20）|
| 跑步时脚部滑动 | 高速时降低 IKAlpha（见 UpdateFootIK 中的逻辑）|
| 骨盆上下颠簸 | 骨盆插值速度调低（Speed=5~8）|
| 斜坡上脚旋转过大 | 限制旋转角度（ClampAngle）|

---

## 六、延伸阅读

- 📄 [代码示例：完整脚步 IK 组件](./code/01_foot_ik_component.cpp)
- 🔗 [Foot Placement 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/foot-placement-in-unreal-engine)
- 🔗 [UE5 脚步 IK 实战（知乎）](https://zhuanlan.zhihu.com/p/381967985)
