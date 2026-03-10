# 11.1 MetaHuman 架构总览

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、MetaHuman 是什么

```
MetaHuman 是 Epic 提供的超写实数字人系统：
  MetaHuman Creator：在线工具，可视化创建高质量人类角色
  导出到 UE5 项目，包含完整骨骼 Mesh、材质、Control Rig、AnimBP

特点：
  170+ 个面部骨骼（普通角色约 20 个面部骨骼）
  高质量皮肤/眼睛/头发材质（PBR + Subsurface Scattering）
  完整的身体 + 面部 AnimBP 系统
  内置 ARKit 52 个 BlendShape 面部捕捉支持
  Groom 发丝（高质量头发）
```

---

## 二、MetaHuman 资产结构

```
导入项目后的文件结构：

MetaHumans/
└── [CharacterName]/
    ├── BP_[Name]                    ← 角色蓝图（继承 BP_MetaHuman）
    ├── Face/
    │   ├── [Name]_Face              ← 面部 SkeletalMesh
    │   ├── CTRL_[Name]_FaceFacial   ← 面部 Control Rig
    │   └── ABP_[Name]_FaceFacial    ← 面部 AnimBlueprint
    ├── Body/
    │   ├── [Name]_Body              ← 身体 SkeletalMesh（m_med_nrw_body 等）
    │   └── ABP_[Name]_Body          ← 身体 AnimBlueprint
    ├── m_[body_type]/               ← 身体 Mesh 变体
    └── [Name]_Groom/                ← Groom 头发资产

关键组件层级：
  BP_MetaHuman
  ├── Body (USkeletalMeshComponent)     ← 身体动画
  ├── Face (USkeletalMeshComponent)     ← 面部动画（独立骨骼）
  ├── Legs（可选，用于不同裤子/鞋）
  └── [Hair Groom Components]          ← 发丝/胡须/眉毛
```

---

## 三、面部与身体的分离架构

```
MetaHuman 的身体和面部是两个独立的 SkeletalMesh：

为什么分离？
  面部骨骼极多（170+），驱动机制完全不同（BlendShape + 骨骼）
  身体用普通骨骼动画（走路/跑步/攻击）
  面部用 FACS BlendShape + 骨骼（表情/口型）
  分离后可独立优化 LOD

两个 Mesh 的连接：
  Face Mesh 通过 Leader Pose Component（原 Master Pose）
  跟随 Body Mesh 的 head 骨骼运动
  → 面部 Mesh 自动跟着身体走，不需要手动同步

配置：
  BP_MetaHuman 构造函数中：
  Face->SetLeaderPoseComponent(Body);
```

---

## 四、MetaHuman 动画系统概览

```
身体动画（Body AnimBP）：
  与普通角色 AnimBP 相同
  可以直接使用第三方动画（通过 IK Retargeter 重定向）
  
面部动画（Face AnimBP）：
  驱动 52 个 ARKit BlendShape 变形目标
  + 额外的骨骼控制（眼球、舌头、下巴）
  输入来源：Live Link（实时）或 Animation Sequence（预录）

动画流程：
  实时动捕：
    iPhone（ARKit）→ Live Link → Face AnimBP → 面部骨骼
    动捕设备       → Live Link → Body AnimBP → 身体骨骼
  
  预录动画：
    Motion Capture → IK Retargeter → Body Animation Sequence
    Facial Capture → ARKit Export → Face Animation Sequence
```

---

## 五、延伸阅读

- 📄 [11.2 身体动画](./02-body-animation.md)
- 🔗 [MetaHuman 官方文档](https://dev.epicgames.com/documentation/en-us/metahuman/metahuman-documentation)
- 🔗 [MetaHuman Creator](https://metahuman.unrealengine.com)
