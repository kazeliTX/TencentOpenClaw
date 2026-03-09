# 4.6 Additive 叠加动画

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、Additive 动画原理

Additive（叠加）动画存储的不是绝对姿势，而是相对于基准姿势的**差量（Delta）**：

```
计算方式：
  普通动画：骨骼绝对变换
  Additive：骨骼变换 - 参考姿势变换（= 差量）

应用时：
  Base Pose + Additive × Alpha = 最终姿势

优点：
  • 叠加动画可以叠加在任意基础动画上
  • 允许分离身体不同部分的动画逻辑
  • 多个 Additive 可以同时叠加（权重相加）

典型应用：
  Aim Offset（瞄准方向叠加到运动动画）
  呼吸动画（轻微胸部起伏叠加到任意姿势）
  受击晃动（Hit Reaction 叠加）
  疲劳/受伤表现（叠加轻微弯腰姿势）
  表情（面部表情叠加到全身动画）
```

---

## 二、Additive 动画的三种类型

```
类型 1：No Additive（普通动画）
  存储绝对骨骼变换
  
类型 2：Local Space（本地空间叠加）
  差量相对于父骨骼空间计算
  优点：计算快
  缺点：当角色旋转时，叠加方向可能偏移
  适用：不涉及世界方向的叠加（如呼吸）
  
类型 3：Mesh Space（网格空间叠加，推荐用于 Aim Offset）
  差量相对于 Mesh 组件空间计算
  优点：叠加方向始终相对于 Mesh 正确
  缺点：计算稍慢（需要额外的空间转换）
  适用：Aim Offset（瞄准方向必须用这个）
```

---

## 三、制作呼吸叠加动画

```
步骤（DCC 软件中）：
  1. 角色处于参考姿势（T/A-Pose）
  2. 制作胸部/脊椎轻微起伏动画（约 2~4 秒循环）
  3. 仅修改：spine_01, spine_02, spine_03（胸部骨骼）
  4. 导出为 FBX

UE 导入设置：
  Additive Anim Type: Local Space
  Reference Pose Type: Reference Pose（使用 Skeleton 的参考姿势）

在 AnimGraph 中：
  [任意基础动画]
        │
  [Apply Additive]
    Base Pose: ← 上面的基础动画
    Additive:  ← AS_Breathing（呼吸叠加）
    Alpha:     ← 0.3~0.5（不要太强，不然看起来很奇怪）
        │
  [Output Pose]
```

---

## 四、运行时动态调整叠加权重

```cpp
// 在 AnimInstance 中暴露叠加权重变量

UPROPERTY(BlueprintReadOnly, Category="Additive")
float BreathingAlpha = 0.3f;     // 正常呼吸强度

UPROPERTY(BlueprintReadOnly, Category="Additive")
float InjuredLeanAlpha = 0.0f;   // 受伤前倾程度

// 更新逻辑（NativeUpdateAnimation 中）
void UpdateAdditiveWeights(float Dt)
{
    // 受伤时增强呼吸动作，增加受伤前倾
    float HP = CachedChar->GetHealthPercent(); // 0~1

    float TargetBreathing = FMath::Lerp(0.2f, 0.8f, 1.0f - HP);
    BreathingAlpha = FMath::FInterpTo(BreathingAlpha, TargetBreathing, Dt, 2.0f);

    float TargetLean = (HP < 0.3f) ? FMath::Clamp((0.3f - HP) / 0.3f, 0.0f, 1.0f) : 0.0f;
    InjuredLeanAlpha = FMath::FInterpTo(InjuredLeanAlpha, TargetLean, Dt, 3.0f);
}
```

---

## 五、多层叠加的注意事项

```
叠加顺序很重要！

正确顺序：
  [Base Locomotion]
        │
  [Apply Additive: Breathing]      ← 先叠加呼吸
        │
  [Apply Mesh Space Additive: Aim Offset] ← 再叠加瞄准
        │
  [Output Pose]

错误（Aim Offset 在呼吸之前）：
  瞄准方向会因为呼吸骨骼偏移而轻微抖动

原则：
  影响范围广的叠加（如呼吸）放前面（靠近 Base）
  精确定向的叠加（如 Aim Offset）放后面（靠近 Output）
```

---

## 六、延伸阅读

- 📄 [4.3 Aim Offset 深度解析](./03-aim-offset.md)
- 🔗 [Additive Animation 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/additive-animation-in-unreal-engine)
