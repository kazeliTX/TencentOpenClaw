# 11.6 Control Rig 面部骨骼控制

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 30 分钟

---

## 一、MetaHuman Face Control Rig 架构

```
CTRL_[Name]_FaceFacial 是专门为面部设计的 Control Rig：

包含两类控制器：
  1. FACS 控制器（直接对应 ARKit BlendShape）
     约 52 个，值范围 0.0 ~ 1.0
     驱动 Morph Target（BlendShape 变形目标）
     
  2. 骨骼控制器（精细控制面部骨骼）
     约 100+ 个，值范围和类型各异
     驱动面部 170+ 根骨骼（眼球、眉毛、嘴唇骨骼等）

数据流：
  FACS 值（来自 Live Link 或 AnimBP）
    → CTRL Face Control Rig Forward Solve
       → 根据 FACS 值计算骨骼变换
       → 叠加 BlendShape 变形
    → 输出：面部 Mesh 最终姿势
```

---

## 二、面部 Control Rig 的 Forward Solve 逻辑

```
MetaHuman Face Control Rig 的 Solve 包含三层：

Layer 1：Corrective Shapes（修正变形）
  某些骨骼组合会导致不自然的皮肤变形
  修正 BlendShape 自动激活以修复这些组合
  例：eyeClose + smile 组合时，眼角需要特殊修正

Layer 2：Direct Bone Drive（直接骨骼驱动）
  FACS 值直接映射到骨骼旋转/位移
  例：jawOpen → 下颌骨骼旋转
      eyeBlink → 上眼睑骨骼旋转

Layer 3：Procedural（程序化修正）
  口型：jawOpen 驱动的同时，嘴唇骨骼自动适配
  眼球方向：eyeLookUp/Down/In/Out 同时驱动眼骨+眼睑

通常不需要修改这个 Control Rig（非常复杂！）
除非需要自定义修正 Shape 或扩展面部骨骼
```

---

## 三、程序化表情合成

```
可以通过程序化设置 FACS 值来合成表情：

// 在 Face AnimBP 的 EventGraph 中直接设置 BlendShape 曲线
void UMyFaceAnimInstance::SetExpression(EFaceExpression Expression)
{
    // 设置 Curve 值（映射到 FACS BlendShape）
    switch (Expression)
    {
    case EFaceExpression::Happy:
        SetCurveValue("mouthSmile_L",   0.8f);
        SetCurveValue("mouthSmile_R",   0.8f);
        SetCurveValue("cheekSquint_L",  0.5f);
        SetCurveValue("cheekSquint_R",  0.5f);
        SetCurveValue("eyeSquint_L",    0.3f);
        SetCurveValue("eyeSquint_R",    0.3f);
        break;
    case EFaceExpression::Angry:
        SetCurveValue("browDown_L",     0.7f);
        SetCurveValue("browDown_R",     0.7f);
        SetCurveValue("mouthFrown_L",   0.6f);
        SetCurveValue("mouthFrown_R",   0.6f);
        SetCurveValue("noseSneer_L",    0.4f);
        SetCurveValue("noseSneer_R",    0.4f);
        break;
    // ... 更多表情
    }
}

// 平滑过渡表情（避免突然跳变）
void UMyFaceAnimInstance::BlendToExpression(
    EFaceExpression Target, float Duration)
{
    // 用 FAlphaBlend 在 Duration 秒内插值当前值到目标值
}
```

---

## 四、眼球控制

```
MetaHuman 眼球有专门的骨骼控制：

骨骼：
  FACIAL_C_EyeDriver_L / _R  ← 眼球驱动骨骼
  
控制方式：
  Live Link：eyeLookUp/Down/In/Out BlendShape 值
  → Control Rig 计算眼球旋转角度
  → 设置 Eye 骨骼 Local Rotation

程序化眼球注视：
  // 在 Face Control Rig 的 Forward Solve 中
  GetControlTransform("ctrl_EyeLook_L") → 设为注视目标方向
  SetBoneRotation(FACIAL_C_EyeDriver_L, EyeRotation)

眨眼（程序化）：
  BlinkCycle = Sin(Time × 0.1 × 2π) > 0.95 ? 1.0 : 0.0
  SetCurveValue("eyeBlink_L", BlinkCycle)
  SetCurveValue("eyeBlink_R", BlinkCycle)
  // 加随机偏移防止双眼完全同步
```

---

## 五、延伸阅读

- 📄 [代码示例：面部动画程序化驱动](./code/02_face_anim_driver.cpp)
- 🔗 [MetaHuman Face Control Rig 文档](https://dev.epicgames.com/documentation/en-us/metahuman/animating-metahumans-in-unreal-engine)
