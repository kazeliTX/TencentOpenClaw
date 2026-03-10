# 11.5 ARKit 面部捕捉（iPhone 方案）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、ARKit 面部捕捉原理

```
iPhone（Face ID 机型，X 及以上）内置 TrueDepth 摄像头：
  30,000 个红外点投射到面部
  红外摄像头捕捉反射图案
  神经引擎实时分析 → 输出 52 个 BlendShape 值

精度：
  可区分微小肌肉运动（肌肉单独控制精度）
  延迟：约 30~50ms（iOS → WiFi → UE）
  
适用场景：
  ✅ 独立游戏/小团队（低成本高质量面部捕捉）
  ✅ 实时预览（边拍边看效果）
  ✅ 过场动画录制
  ❌ 不适合极高精度（专业级用 Vicon/光学设备）
```

---

## 二、ARKit BlendShape 到 MetaHuman 的映射

```
iPhone 输出的 52 个 ARKit BlendShape
需要映射到 MetaHuman 的 BlendShape（命名稍有不同）：

UE 的 Apple ARKit 插件自动处理这个映射：
  启用插件后，Live Link Face Subject 直接输出
  符合 MetaHuman 命名规范的 BlendShape 值

Face AnimBP 中的 Live Link Pose 节点：
  使用 ARKit Face Subject
  → 自动将 52 个值映射到 MetaHuman BlendShape
  → 无需手动配置映射

自定义映射（如果角色不是 MetaHuman）：
  创建 Live Link Remap Asset
  手动指定每个 ARKit 值对应哪个 BlendShape/骨骼
```

---

## 三、后处理：减少捕捉抖动

```
ARKit 原始数据有高频噪声（头部轻微抖动）

AnimBP 中的后处理策略：

1. BlendShape 平滑（Low-Pass Filter）
   对每个 BlendShape 值做低通滤波：
   FilteredValue = Lerp(PreviousValue, RawValue, SmoothAlpha)
   SmoothAlpha = 0.3~0.6（越低越平滑，越慢）
   
2. 骨骼旋转平滑
   头部旋转（Pitch/Yaw/Roll）用 RInterpTo 平滑

3. 关键值阈值过滤
   小于 0.02 的 BlendShape 值归零（消除微小抖动）
   避免面部持续轻微"抽搐"

4. Sequencer 后处理
   录制后在 Sequencer 曲线编辑器：
   选中曲线 → 平滑（Bake + Smooth Filter）
   → 批量平滑所有 BlendShape 曲线
```

---

## 四、实战技巧

```
提升 ARKit 捕捉质量：
  ✅ 好的光线（均匀正面光，减少阴影干扰）
  ✅ 手机固定（用手机支架，减少手抖）
  ✅ 演员和手机保持约 40~60cm 距离
  ✅ 录制前做校准（Live Link Face App → Calibrate）
  
提升表演质量：
  ✅ 演员用声音驱动表情（说台词，而非只做表情）
  ✅ 放大夸张表情（摄像机会压缩细节）
  ✅ 保持眼神接触（看手机摄像头，而非屏幕）

常见问题：
  眼球不同步 → 检查 eyeLookUp/Down/In/Out 映射
  嘴唇穿插 → 降低 jawOpen 的最大映射值
  整体延迟感 → 在 Live Link Subject 中降低 Interpolation
```

---

## 五、延伸阅读

- 🔗 [Apple ARKit Face Capture 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/live-link-face-in-unreal-engine)
- 🔗 [Live Link Face App（App Store）](https://apps.apple.com/us/app/live-link-face/id1495370836)
