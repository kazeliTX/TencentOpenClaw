# 2.8 运行时骨骼变换修改

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、运行时修改骨骼的方式概览

```
方式                        使用场景                         优先推荐度
─────────────────────────────────────────────────────────────────────
AnimGraph 节点（蓝图/C++）  AnimBP 内的程序化控制              ⭐⭐⭐⭐⭐
Transform Modify Bone       单根骨骼简单变换                   ⭐⭐⭐⭐
Skeletal Control            通用骨骼控制节点库                  ⭐⭐⭐⭐
Custom AnimNode（C++）      高性能自定义节点                    ⭐⭐⭐⭐
BoneTransform（直接写入）   调试用，不推荐正式使用              ⭐⭐
```

---

## 二、Transform Modify Bone 节点

这是 AnimGraph 中最简单的骨骼变换修改方式：

```
在 AnimBP AnimGraph 中使用：
  搜索："Transform (Modify) Bone"

参数设置：
  Bone to Modify: spine_03        （要修改的骨骼）
  
  Translation:
    Translation Mode: Add to Existing  （叠加/替换/忽略）
    Translation: (X=0, Y=0, Z=10)     （位移量，cm）
    Translation Space: Bone Space      （参考空间）
  
  Rotation:
    Rotation Mode: Replace Existing
    Rotation: (Pitch=15, Yaw=0, Roll=0)（旋转量）
    Rotation Space: Component Space
  
  Scale:
    Scale Mode: Ignore                 （不修改缩放）
```

### 通过 C++ 驱动 Transform Modify Bone

```cpp
// 在 AnimInstance 中暴露变量给 AnimGraph 节点
UPROPERTY(BlueprintReadWrite, Category = "BoneControl")
FRotator SpineExtraRotation = FRotator::ZeroRotator;

UPROPERTY(BlueprintReadWrite, Category = "BoneControl")
FVector HeadLookAtOffset = FVector::ZeroVector;

// 在 NativeThreadSafeUpdateAnimation 中更新
virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override
{
    // 计算脊椎弯曲（如：根据相机俯仰让角色身体前倾）
    float PitchDeg = FMath::ClampAngle(CachedAimPitch * 0.3f, -15.0f, 20.0f);
    SpineExtraRotation = FRotator(PitchDeg, 0.0f, 0.0f);
}
```

---

## 三、Skeletal Control 节点库

| 节点名 | 功能 |
|-------|------|
| `Look At` | 使骨骼始终朝向目标位置 |
| `Two Bone IK` | 双骨骼 IK（手臂、腿部）|
| `FABRIK` | 链式 IK |
| `Bone Driven Controller` | 用一根骨骼驱动另一根 |
| `Spring Controller` | 弹簧追踪效果（次级运动）|
| `Rigid Body` | 骨骼物理模拟节点 |
| `Copy Bone` | 将一根骨骼的变换复制到另一根 |
| `Modify Curve` | 在 AnimGraph 中修改曲线值 |

### Look At 节点配置

```
用途：让角色头部/眼睛朝向目标（如注视玩家）

Bone to Modify:   head
Look At Target:   (目标位置，Component Space)
Look At Axis:     X (头部正前方)
Look At Clamp:    70° (最大转头角度)
Interpolation Speed: 5.0 (平滑速度)

在 AnimBP 中配合变量：
  // AnimInstance 中
  UPROPERTY(BlueprintReadWrite)
  FVector LookAtTargetCS;  // Component Space 目标点

  // NativeUpdateAnimation 中更新
  FVector WorldTarget = GetLookAtTarget()->GetActorLocation();
  LookAtTargetCS = Mesh->GetComponentTransform()
      .InverseTransformPosition(WorldTarget);
```

---

## 四、自定义 AnimNode（C++）

对于性能关键的骨骼控制，推荐实现自定义 AnimNode：

```cpp
// ── AnimNode（工作线程运行）──

// MyAnimNode_HeadLookAt.h
USTRUCT(BlueprintInternalUseOnly)
struct FAnimNode_HeadLookAt : public FAnimNode_SkeletalControlBase
{
    GENERATED_BODY()

    // 目标骨骼
    UPROPERTY(EditAnywhere, Category = Settings)
    FBoneReference BoneToModify;

    // 从 AnimInstance 传入的目标点（Component Space）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings,
              meta = (PinShownByDefault))
    FVector LookAtTarget = FVector::ForwardVector * 100.0f;

    // 最大旋转角度
    UPROPERTY(EditAnywhere, Category = Settings)
    float MaxAngle = 70.0f;

    // 平滑速度
    UPROPERTY(EditAnywhere, Category = Settings)
    float InterpSpeed = 5.0f;

private:
    FQuat CurrentRotation = FQuat::Identity;

public:
    // 工作线程中执行（线程安全）
    virtual void EvaluateSkeletalControl_AnyThread(
        FComponentSpacePoseContext& Output,
        TArray<FBoneTransform>& OutBoneTransforms) override;

    virtual bool IsValidToEvaluate(
        const USkeleton* Skeleton,
        const FBoneContainer& RequiredBones) override;

    virtual void InitializeBoneReferences(
        const FBoneContainer& RequiredBones) override;
};

// MyAnimNode_HeadLookAt.cpp
void FAnimNode_HeadLookAt::EvaluateSkeletalControl_AnyThread(
    FComponentSpacePoseContext& Output,
    TArray<FBoneTransform>& OutBoneTransforms)
{
    check(OutBoneTransforms.Num() == 0);

    const FBoneContainer& BoneContainer = Output.Pose.GetPose().GetBoneContainer();
    FCompactPoseBoneIndex BoneIdx = BoneToModify.GetCompactPoseIndex(BoneContainer);
    if (BoneIdx == INDEX_NONE) return;

    // 获取当前骨骼 Component Space 变换
    FTransform BoneCSTransform = Output.Pose.GetComponentSpaceTransform(BoneIdx);

    // 计算需要旋转的方向（从骨骼指向目标）
    FVector CurrentForward = BoneCSTransform.GetRotation().GetForwardVector();
    FVector ToTarget = (LookAtTarget - BoneCSTransform.GetLocation()).GetSafeNormal();

    // 计算旋转角度，限制最大值
    float Angle = FMath::RadiansToDegrees(
        FMath::Acos(FVector::DotProduct(CurrentForward, ToTarget)));
    float ClampedAngle = FMath::Min(Angle, MaxAngle);

    // 计算旋转轴和目标四元数
    FVector RotAxis = FVector::CrossProduct(CurrentForward, ToTarget).GetSafeNormal();
    FQuat TargetRot = FQuat(RotAxis, FMath::DegreesToRadians(ClampedAngle))
        * BoneCSTransform.GetRotation();

    // 平滑插值（DeltaTime 通过上下文获取）
    float DeltaTime = Output.AnimInstanceProxy->GetDeltaSeconds();
    CurrentRotation = FQuat::Slerp(
        CurrentRotation,
        TargetRot,
        FMath::Min(InterpSpeed * DeltaTime, 1.0f)
    );
    CurrentRotation.Normalize();

    // 输出变换
    FTransform NewTransform = BoneCSTransform;
    NewTransform.SetRotation(CurrentRotation);
    OutBoneTransforms.Add(FBoneTransform(BoneIdx, NewTransform));
}
```

---

## 五、直接设置骨骼变换（调试用）

```cpp
// ⚠️ 警告：以下方法会在下一帧被 AnimBP 覆盖，仅用于调试！

// 临时设置骨骼位置（在 Tick 中持续调用才能维持效果）
void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 仅作调试示例！
    int32 BoneIdx = GetMesh()->GetBoneIndex(FName("head"));
    if (BoneIdx != INDEX_NONE)
    {
        // 这种方式不被推荐在正式项目中使用
        // 应改为：通过 AnimBP 变量 → AnimGraph 节点 → 骨骼变换
    }
}
```

---

## 六、延伸阅读

- 🔗 [Skeletal Control 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/skeletal-controls-in-unreal-engine)
- 🔗 [Custom AnimNode 教程](https://dev.epicgames.com/community/learning/tutorials/d68K/unreal-engine-animation-node-in-c)
- 🔗 [Transform Modify Bones（完整指南）](https://www.unreal-university.blog/how-to-transform-modify-bones-in-unreal-engine-5-complete-guide/)
- 📄 [代码示例：骨骼变换控制器](./code/01_bone_modifier.cpp)
