# 10.9 Control Rig + Sequencer 动画制作

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 20 分钟

---

## 一、Sequencer 中使用 Control Rig

```
Control Rig 的控制器可以在 Sequencer 中直接 K 帧：

工作流：
  1. 在关卡中放置角色
  2. 打开 Sequencer
  3. 将角色拖入 Sequencer
  4. 在角色轨道下：+ Animation → Control Rig
     选择：CR_MyCharacter
  5. 视口中出现控制器 Gizmo（圆圈/方块等）
  6. 选中控制器 → S 键记录当前变换 → 创建关键帧
  7. 移动时间轴 → 调整控制器位置 → 再次 S 键

优势：
  实时预览 IK 效果（脚踩地面/手握物体）
  与程序化动画叠加（Control Rig 基础 + 手动微调）
  动画可导出为 Animation Sequence
```

---

## 二、Bake to Animation Sequence

```
将 Control Rig 动画导出为 Animation Sequence：

Sequencer → 选中 Control Rig 轨道
→ Control Rig → Bake To Animation Sequence
→ 选择输出路径和命名
→ 生成独立的 .uasset 动画文件

应用：
  过场动画最终打包
  动画师完成动画后交给程序员使用
  运行时不需要 Control Rig（只用预烘焙的动画）
```

---

## 三、Control Rig + 全身 IK 过场动画

```
Sequencer 过场动画实战建议：

层级（从底到上）：
  Layer 1：基础运动（走路/转身）← Motion Capture 或手 K
  Layer 2：脚部 IK（贴合地面） ← Control Rig Two Bone IK
  Layer 3：手部 IK（握物体）   ← Control Rig FABRIK
  Layer 4：面部动画             ← Live Link / MetaHuman
  Layer 5：细节微调             ← 手动 K 帧覆盖

Sequencer 轨道结构：
  Character
  ├── Animation (Walk_Animation)
  ├── Control Rig (CR_Character)
  │   ├── ctrl_foot_l [Key Frames]
  │   ├── ctrl_foot_r [Key Frames]
  │   └── ctrl_hand_l [Key Frames]
  └── Face Animation (LiveLink)
```

---

## 四、延伸阅读

- 🔗 [Sequencer + Control Rig 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-in-sequencer-in-unreal-engine)
- 🔗 [Bake to Animation Sequence](https://dev.epicgames.com/documentation/en-us/unreal-engine/baking-control-rig-to-animation-sequence-in-unreal-engine)
