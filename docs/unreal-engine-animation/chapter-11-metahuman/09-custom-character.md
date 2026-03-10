# 11.9 自定义角色接入 MetaHuman 动画系统

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 30 分钟

---

## 一、为什么要接入 MetaHuman 系统

```
场景：
  你有一个自定义角色（非 MetaHuman Creator 生成）
  但想使用 MetaHuman 的面部捕捉系统（ARKit + Live Link）
  或想使用 MetaHuman 的 Control Rig 面部动画工具

方案：
  将自定义角色的骨骼映射到 MetaHuman 面部骨骼规范
  使用相同的 Face AnimBP（ABP_MetaHuman_FaceFacial）
  → 无需重新开发面部动画系统
```

---

## 二、骨骼兼容性要求

```
MetaHuman 面部骨骼规范（关键骨骼必须存在）：

下颌：
  FACIAL_C_Jaw

眼球：
  FACIAL_L_Eye / FACIAL_R_Eye
  FACIAL_L_EyeDriver / FACIAL_R_EyeDriver

眼睑：
  FACIAL_L_UpperLid / FACIAL_R_UpperLid
  FACIAL_L_LowerLid / FACIAL_R_LowerLid

嘴唇（关键）：
  FACIAL_L_LipCorner / FACIAL_R_LipCorner
  FACIAL_C_LipUpper / FACIAL_C_LipLower

眉毛：
  FACIAL_L_BrowInner / FACIAL_R_BrowInner
  FACIAL_L_BrowOuter / FACIAL_R_BrowOuter

如果自定义骨骼有不同命名：
  创建 Bone Rename 映射（在 IK Retargeter 中配置）
  或直接重命名 DCC 中的骨骼（推荐，一劳永逸）
```

---

## 三、BlendShape 兼容性

```
自定义角色需要有 52 个 ARKit 标准 BlendShape：
  命名规范：与 ARKit 完全一致（见 11.3 节 BlendShape 表）
  
在 Maya/Blender 中制作：
  基于角色的面部拓扑创建 52 个变形目标
  按 ARKit 规范命名
  导出到 UE（FBX 导入时携带 Morph Target）

使用 MetaHuman Mesh 但自定义外观：
  从 MetaHuman Creator 导出基础 Mesh
  在 ZBrush/Maya 中修改外观（但保留 BlendShape 结构）
  → 最简单的方式：保证 BlendShape 和骨骼完整
```

---

## 四、集成步骤（快速路线）

```
最快接入 MetaHuman 面部系统的方法：

方法：使用 MetaHuman 面部 Mesh 替换
  1. 从 MetaHuman Creator 下载任意 MetaHuman
  2. 将 Face Mesh 替换为自己的风格化面部
     （但保持骨骼和 BlendShape 不变）
  3. 直接使用 CTRL_[Name]_FaceFacial Control Rig
  4. 使用 ABP_MetaHuman_FaceFacial AnimBP

适合场景：
  风格化人类角色（卡通/半写实）想要面部捕捉
  外观可以自定义，但骨骼保持 MetaHuman 规范

完全自定义骨骼的接入：
  更复杂：需要自建 Face Control Rig
  参考 MetaHuman Control Rig 的结构自行构建
  工作量：约 1~2 周（有 MetaHuman 参考的情况下）
```

---

## 五、延伸阅读

- 🔗 [MetaHuman SDK（自定义角色集成）](https://dev.epicgames.com/documentation/en-us/metahuman/metahuman-sdk)
- 🔗 [MetaHuman Creator 文档](https://dev.epicgames.com/documentation/en-us/metahuman/metahuman-creator-documentation)
