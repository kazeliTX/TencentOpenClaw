# 4.4 Animation Montage 完全指南

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 40 分钟
> Montage 是实现技能、攻击、互动等一次性动作的核心工具

---

## 一、Montage 是什么

Animation Montage（动画蒙太奇）是一种特殊的动画资产，在 State Machine **之外**独立播放，用于：

```
Montage 的核心特性：
  1. 独立播放：不依赖 State Machine，通过 C++/蓝图直接触发
  2. Slot 系统：可以选择性地覆盖身体的某一部分（上半身/全身）
  3. 分段（Section）：一个 Montage 可以包含多个段落，可跳转
  4. 混合树（Branch Point）：在特定帧做精确的逻辑判断
  5. 通知（Notify）：内置通知轨道，触发脚步声/攻击判定等
  6. 根运动（Root Motion）：支持位移驱动

典型使用场景：
  攻击连段（普攻 → 连击 → 必杀）
  技能动作（释放技能时播放全身动画）
  互动动作（开门、拾取、爬梯）
  表情/动作（NPC 对话手势）
  受击动画（Hit Reaction）
```

---

## 二、Montage 结构详解

```
Montage 时间轴结构：

时间 →  0    0.3   0.8   1.2   1.6   2.0
         │     │     │     │     │     │
Section: [─ Start ─────][─ Loop ─────][─ End ─]
         攻击起手          循环挥击       收招
         
轨道：
  Slot:     [────────── UpperBody ────────────]
  Notifies: [AttackBegin↑] [AttackEnd↑] [SFX↑]
  Curves:   [──── MontageBlendAlpha ──────────]
  
关键组成部分：
  • Sections（段落）：可命名的时间区段，支持跳转
  • Slots（槽位）：指定动画覆盖哪个身体区域
  • Notifies（通知）：时间点事件
  • Curves（曲线）：随时间变化的浮点值
```

---

## 三、创建 Montage

```
1. 内容浏览器 → 右键 → Animation → Animation Montage
   选择 Skeleton → 命名：AM_Attack_Light

2. 将动画序列拖入 Montage 时间轴

3. 配置 Slot（槽位）：
   轨道上点击 Slot 下拉 → 选择 "FullBody" 或 "UpperBody"
   （Slot 必须在 AnimGraph 中有对应的 Slot 节点才能播放）

4. 添加 Section（段落）：
   时间轴上右键 → New Montage Section
   命名：Start、Loop、End
   
5. 设置 Section 循环：
   Montage Sections 面板 → Loop 段 → 右键 → Loop（让 Loop 段循环）

6. 添加 Notify（见第三章 Notify 系统）
```

---

## 四、C++ 播放 Montage

```cpp
// 播放 Montage 的完整示例
void AMyCharacter::PlayAttackMontage(UAnimMontage* MontageToPlay, float PlayRate)
{
    if (!MontageToPlay) return;

    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!AnimInst) return;

    // 播放 Montage（返回播放时长）
    float Duration = AnimInst->Montage_Play(MontageToPlay, PlayRate);

    if (Duration > 0.0f)
    {
        // 绑定完成回调
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AMyCharacter::OnAttackMontageEnded);
        AnimInst->Montage_SetEndDelegate(EndDelegate, MontageToPlay);

        UE_LOG(LogTemp, Log, TEXT("Playing montage: %s (%.2fs)"),
            *MontageToPlay->GetName(), Duration);
    }
}

// Montage 结束回调
void AMyCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        UE_LOG(LogTemp, Log, TEXT("Montage completed normally"));
        OnAttackFinished.Broadcast();   // 通知游戏逻辑攻击结束
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Montage interrupted"));
    }
}

// 跳转到指定 Section
void AMyCharacter::JumpToMontageSection(UAnimMontage* Montage, FName SectionName)
{
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->Montage_JumpToSection(SectionName, Montage);
    }
}

// 停止 Montage（带混合时长）
void AMyCharacter::StopCurrentMontage(float BlendOutTime = 0.25f)
{
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->Montage_Stop(BlendOutTime);
    }
}

// 暂停 Montage
void AMyCharacter::PauseMontage(UAnimMontage* Montage)
{
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        AnimInst->Montage_Pause(Montage);
    }
}

// 查询 Montage 是否正在播放
bool AMyCharacter::IsMontageActive(UAnimMontage* Montage) const
{
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!AnimInst) return false;
    return AnimInst->Montage_IsActive(Montage);
}

// 获取当前 Section 名称
FName AMyCharacter::GetCurrentMontageSection(UAnimMontage* Montage) const
{
    UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
    if (!AnimInst) return NAME_None;
    return AnimInst->Montage_GetCurrentSection(Montage);
}
```

---

## 五、Montage 连段系统

```cpp
// 连击系统示例：通过跳转 Section 实现攻击连段

class UComboAttackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    /** 执行连击（调用一次 = 请求下一段）*/
    UFUNCTION(BlueprintCallable)
    void ExecuteCombo()
    {
        if (!CachedChar || !CombatMontage) return;

        UAnimInstance* Anim = CachedChar->GetMesh()->GetAnimInstance();

        if (!Anim->Montage_IsActive(CombatMontage))
        {
            // 第一段：从 Attack1 开始
            CurrentComboIndex = 0;
            Anim->Montage_Play(CombatMontage);
            Anim->Montage_JumpToSection(FName("Attack1"), CombatMontage);
        }
        else if (bComboWindowOpen)
        {
            // 连击窗口内请求下一段
            CurrentComboIndex = FMath::Min(CurrentComboIndex + 1, MaxComboCount - 1);
            bComboRequested = true;
        }
    }

    /** 在 AnimNotify 中调用：打开连击窗口 */
    void OnComboWindowOpen()  { bComboWindowOpen = true; }
    void OnComboWindowClose()
    {
        if (bComboRequested)
        {
            // 跳转到下一段
            FName NextSection = FName(*FString::Printf(TEXT("Attack%d"), CurrentComboIndex + 1));
            CachedChar->GetMesh()->GetAnimInstance()->Montage_JumpToSection(
                NextSection, CombatMontage);
        }
        bComboWindowOpen = false;
        bComboRequested  = false;
    }

private:
    UPROPERTY() TObjectPtr<ACharacter> CachedChar;
    UPROPERTY(EditAnywhere) TObjectPtr<UAnimMontage> CombatMontage;

    int32 CurrentComboIndex = 0;
    int32 MaxComboCount     = 3;
    bool bComboWindowOpen   = false;
    bool bComboRequested    = false;
};
```

---

## 六、Branch Point vs AnimNotify

```
Branch Point（分支点）：
  精确时间点触发（同步，Game Thread 内）
  适合：需要精确在某帧触发逻辑（如物理碰撞）
  
AnimNotify：
  异步触发（Worker Thread 内，有 1 帧延迟）
  适合：大多数游戏逻辑（音效、特效、状态切换）
  
选择原则：
  99% 的情况用 AnimNotify（性能更好）
  只有需要完全同步、零延迟时才用 Branch Point
```

---

## 七、延伸阅读

- 📄 [4.5 Montage Slot 与混合层](./05-montage-slots.md)
- 🔗 [Animation Montage 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-montage-in-unreal-engine)
- 📄 [代码示例：Montage 管理器](./code/01_montage_manager.cpp)
