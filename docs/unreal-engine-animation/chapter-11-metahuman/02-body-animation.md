# 11.2 MetaHuman 身体动画系统

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、Body AnimBlueprint 结构

```
MetaHuman 自带的 ABP_[Name]_Body 结构：

EventGraph：
  读取 CharacterMovement 速度/方向
  读取 AimPitch/AimYaw
  更新 BlendSpace 参数

AnimGraph：
  [Locomotion State Machine]
    ← 行走/跑步/跳跃（可替换为自定义 SM）
          │
  [Upper Body Layer]（上半身叠加）
          │
  [Aim Offset]（瞄准偏移）
          │
  [Hand IK]（双手 IK）
          │
  [Foot IK]（脚步地面适配）
          │
  [Output Pose]

与自定义角色 AnimBP 几乎相同
主要区别：骨骼命名不同（MetaHuman 用 UE5 标准命名）
```

---

## 二、替换身体动画

```
MetaHuman 使用 UE5 Mannequin 骨骼（标准骨骼）：
  骨骼命名完全符合 UE5 标准
  → 可直接使用 UE Marketplace 上的 UE5 动画！

替换步骤：
  1. 打开 ABP_[Name]_Body
  2. 找到 Locomotion State Machine → 替换为自己的
  3. 或直接替换整个 AnimBP（更激进的方案）

不需要 IK Retargeter 的情况：
  MetaHuman + UE5 Mannequin 动画 → 直接兼容（骨骼相同）
  
需要 IK Retargeter 的情况：
  Mixamo 动画 → 需要重定向（不同骨骼命名）
  UE4 旧版动画 → 需要重定向（不同骨骼比例）
```

---

## 三、身体 LOD 与 Mesh 切换

```
MetaHuman 身体有多套 LOD Mesh：
  LOD 0: 完整 Mesh（高质量，高多边形）
  LOD 1: 中等质量
  LOD 2: 低质量（远景用）

不同体型的 Mesh：
  m_med_nrw（中等身材/标准）
  m_tal_fat（高大/肥胖）
  等多种体型

在 BP_MetaHuman 中切换体型：
  Body Component → Skeletal Mesh: 选择对应体型 Mesh
  确保 AnimBP 与新体型 Mesh 的骨骼兼容（通常都用相同骨骼）
```

---

## 四、与 IK Retargeter 集成

```
将第三方动画重定向到 MetaHuman：

步骤：
  1. 为第三方角色创建 IK Rig（IKRig_Source）
  2. 为 MetaHuman Body 创建 IK Rig（IKRig_MetaHuman）
     注意：MetaHuman 提供官方 IK Rig 模板（推荐使用）
  3. 创建 IK Retargeter：Source → MetaHuman
  4. 在 Retargeter 中对齐 T-Pose（MetaHuman 默认 T-Pose）
  5. 导出重定向动画

官方 MetaHuman IK Rig：
  路径：MetaHumans/Common/Body/IKRig_MetaHuman_Body
  已预配置好所有骨骼链，直接用即可
```

---

## 五、延伸阅读

- 📄 [11.3 面部动画系统](./03-face-animation.md)
- 🔗 [MetaHuman 动画文档](https://dev.epicgames.com/documentation/en-us/metahuman/animating-metahumans-in-unreal-engine)
