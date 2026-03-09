# 3.9 Animation Notify 系统

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、Notify 系统概述

Animation Notify 允许在动画播放的**特定时间点**触发逻辑：

```
Notify 分类：

1. AnimNotify（点通知）
   特定帧触发，执行一次
   用途：脚步声、武器特效启用、攻击判定开始

2. AnimNotifyState（区间通知）
   有开始（NotifyBegin）和结束（NotifyEnd）
   整个区间内每帧调用 NotifyTick
   用途：攻击判定区间、特效持续时间、音效循环

3. Notify Track（通知轨道）
   动画编辑器时间轴上的横条
   可以添加多条轨道（如：FootL, FootR, FX, Audio）
```

---

## 二、内置 Notify 类型

| Notify | 说明 |
|--------|------|
| `Play Sound` | 在指定帧播放音效（带空间位置）|
| `Play Particle Effect` | 播放粒子特效（如脚步尘土）|
| `Play Niagara Effect` | 播放 Niagara 特效 |
| `Enable Cloth Physics` | 开启布料模拟 |
| `Disable Cloth Physics` | 关闭布料模拟 |
| `Rigid Body` | 切换 Rigid Body 节点状态 |
| `Set Footstep Override` | 覆盖脚步音效材质 |
| `Trail` | 武器拖尾特效（区间）|

---

## 三、自定义 AnimNotify（C++）

```cpp
// ─────────────────────────────────────────────────
// AnimNotify_MeleeHit.h
// 近战攻击判定开始通知
// ─────────────────────────────────────────────────
#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_MeleeHit.generated.h"

UCLASS(meta=(DisplayName="Melee Hit Begin"))
class MYPROJECT_API UAnimNotify_MeleeHit : public UAnimNotify
{
    GENERATED_BODY()

public:
    // 通知显示名称（在动画编辑器中）
    virtual FString GetNotifyName_Implementation() const override
    {
        return FString::Printf(TEXT("MeleeHit[%s]"), *WeaponSocket.ToString());
    }

    // 核心回调
    virtual void Notify(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override
    {
        if (!MeshComp) return;

        // 找到角色并通知开始攻击判定
        ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
        if (!Character) return;

        // 调用角色的攻击判定接口
        // 实际项目中建议通过接口（IAttackInterface）调用，避免强耦合
        if (UMeleeAttackComponent* AttackComp =
            Character->FindComponentByClass<UMeleeAttackComponent>())
        {
            AttackComp->BeginHitDetection(WeaponSocket, HitRadius);
        }
    }

public:
    /** 武器 Socket 名称（碰撞检测起点）*/
    UPROPERTY(EditAnywhere, Category = "Melee")
    FName WeaponSocket = FName("weapon_muzzle");

    /** 命中判定半径（cm）*/
    UPROPERTY(EditAnywhere, Category = "Melee")
    float HitRadius = 30.0f;
};

// ─────────────────────────────────────────────────
// AnimNotifyState_AttackWindow.h
// 攻击判定窗口（区间通知）
// ─────────────────────────────────────────────────
UCLASS(meta=(DisplayName="Attack Window"))
class MYPROJECT_API UAnimNotifyState_AttackWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    // 区间开始
    virtual void NotifyBegin(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float TotalDuration,
        const FAnimNotifyEventReference& EventReference) override
    {
        UE_LOG(LogTemp, Log, TEXT("Attack Window OPEN (%.2f sec)"), TotalDuration);
        if (auto* Char = Cast<AMyCharacter>(MeshComp->GetOwner()))
            Char->SetAttackWindowActive(true);
    }

    // 区间结束
    virtual void NotifyEnd(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override
    {
        UE_LOG(LogTemp, Log, TEXT("Attack Window CLOSED"));
        if (auto* Char = Cast<AMyCharacter>(MeshComp->GetOwner()))
            Char->SetAttackWindowActive(false);
    }

    // 每帧回调（区间内）
    virtual void NotifyTick(
        USkeletalMeshComponent* MeshComp,
        UAnimSequenceBase* Animation,
        float FrameDeltaTime,
        const FAnimNotifyEventReference& EventReference) override
    {
        // 可在此每帧做碰撞扫描
    }
};
```

---

## 四、脚步音效 Notify 实践

```
脚步系统是 AnimNotify 最典型的应用：

1. 在动画编辑器中：
   打开 Walk/Run 动画 → Notify 轨道
   在左脚落地帧添加：FootLeft Notify
   在右脚落地帧添加：FootRight Notify

2. 自定义 AnimNotify_Footstep：
   Notify() 中：
     获取脚骨骼的世界位置
     向下做 Line Trace（检测地表材质）
     根据材质播放对应音效（土地/金属/水/草地）
     生成脚步粒子（泥地溅起/水花）

3. 材质音效映射（在 DataAsset 中）：
   PhysMat_Dirt   → Sound_FootstepDirt + VFX_DustPuff
   PhysMat_Metal  → Sound_FootstepMetal
   PhysMat_Water  → Sound_FootstepWater + VFX_Splash
```

---

## 五、AnimNotify 在 AnimBP 中监听

```cpp
// 方法 1：在 AnimBP 的 Event Graph 中接收 Notify 事件
// 内置节点：AnimNotify Events → 选择你的 Notify 类
// 这是蓝图的最简单方式

// 方法 2：C++ 中通过委托监听（更灵活）
void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 注册 Notify 回调
    UAnimInstance* Anim = GetMesh()->GetAnimInstance();
    if (Anim)
    {
        Anim->OnPlayMontageNotifyBegin.AddDynamic(
            this,
            &AMyCharacter::OnMontageNotifyBegin
        );
    }
}

void AMyCharacter::OnMontageNotifyBegin(
    FName NotifyName,
    const FBranchingPointNotifyPayload& Payload)
{
    if (NotifyName == FName("AttackWindowOpen"))
    {
        SetAttackWindowActive(true);
    }
    else if (NotifyName == FName("AttackWindowClose"))
    {
        SetAttackWindowActive(false);
    }
}
```

---

## 六、延伸阅读

- 🔗 [Animation Notify 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-notifies-in-unreal-engine)
- 🔗 [自定义 AnimNotify 教程](https://dev.epicgames.com/community/learning/tutorials/qlYB/unreal-engine-custom-animation-notify)
- 📄 [代码示例：自定义 Notify](./code/03_custom_notify.cpp)
