# 11.3 面部动画系统（ARKit / FACS）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、面部动画驱动机制

```
MetaHuman 面部动画由两层驱动：

层 1：FACS BlendShape（Morph Target）
  FACS = Facial Action Coding System（面部动作编码系统）
  52 个 ARKit 标准 BlendShape（与 iPhone ARKit 兼容）
  每个 BlendShape 控制一种面部肌肉运动（0.0 ~ 1.0）
  
  关键 BlendShape：
    browInnerUp       ← 内眉上挑
    browDown_L/R      ← 眉毛下压
    eyeWide_L/R       ← 眼睛睁大
    eyeBlink_L/R      ← 眨眼
    mouthSmile_L/R    ← 微笑
    jawOpen           ← 张嘴
    tongueOut         ← 吐舌头
    cheekPuff         ← 鼓腮

层 2：面部骨骼（骨骼精细控制）
  170+ 根骨骼（眼球旋转、下颌、舌骨等）
  在 BlendShape 基础上叠加骨骼变形
  Control Rig 驱动（CTRL_[Name]_FaceFacial）
```

---

## 二、ARKit 52 BlendShape 完整表

```
眉毛区域（6个）：
  browDown_L / browDown_R          ← 眉毛下压
  browInnerUp                      ← 内眉上挑（担忧）
  browOuterUp_L / browOuterUp_R    ← 外眉上挑（惊讶）

眼睛区域（14个）：
  eyeBlink_L / eyeBlink_R          ← 眨眼
  eyeLookDown_L / eyeLookDown_R    ← 向下看
  eyeLookIn_L / eyeLookIn_R        ← 向内看（斗鸡眼）
  eyeLookOut_L / eyeLookOut_R      ← 向外看
  eyeLookUp_L / eyeLookUp_R        ← 向上看
  eyeSquint_L / eyeSquint_R        ← 眯眼
  eyeWide_L / eyeWide_R            ← 瞪眼

鼻子区域（2个）：
  noseSneer_L / noseSneer_R        ← 鼻子皱缩（嫌弃）

面颊区域（3个）：
  cheekPuff                        ← 鼓腮
  cheekSquint_L / cheekSquint_R    ← 面颊上提（笑）

嘴巴区域（20个）：
  jawForward / jawLeft / jawRight  ← 下颌运动
  jawOpen                          ← 张嘴
  mouthClose                       ← 嘴唇闭合
  mouthFunnel                      ← 嘴唇收拢（吹口哨）
  mouthPucker                      ← 噘嘴
  mouthLeft / mouthRight           ← 嘴向左/右
  mouthSmile_L / mouthSmile_R      ← 微笑
  mouthFrown_L / mouthFrown_R      ← 皱眉（嘴角下垂）
  mouthDimple_L / mouthDimple_R    ← 酒窝
  mouthStretch_L / mouthStretch_R  ← 嘴角外拉
  mouthRollLower / mouthRollUpper  ← 嘴唇内卷
  mouthShrugLower / mouthShrugUpper← 嘴唇耸动
  mouthPress_L / mouthPress_R      ← 嘴角按压

其他（7个）：
  tongueOut                        ← 吐舌头
  neckSlide / neckBack             ← 颈部动作（MetaHuman 扩展）
```

---

## 三、Face AnimBlueprint 配置

```
ABP_[Name]_FaceFacial 结构：

输入来源（三选一）：
  1. Live Link（实时面部捕捉）
  2. Animation Sequence（预录动画）
  3. 程序化（C++ / 蓝图驱动 BlendShape 值）

AnimGraph：
  [Live Link Pose（Face Subject）]
  或
  [Animation Sequence Player]
        │
  [Modify Curve（后处理 BlendShape 值）]
        │
  [Control Rig（CTRL_[Name]_FaceFacial）]
        │
  [Output Pose]

Live Link 数据格式：
  SubjectName: iPhone_Face（在 Live Link 窗口中设置）
  数据：52 个 BlendShape 值 + 头部旋转
  直接映射到 Face AnimBP
```

---

## 四、延伸阅读

- 📄 [11.4 Live Link 实时捕捉](./04-live-link.md)
- 📄 [代码示例：面部动画程序化驱动](./code/02_face_anim_driver.cpp)
- 🔗 [MetaHuman 面部动画文档](https://dev.epicgames.com/documentation/en-us/metahuman/facial-animation-in-metahuman)
