# 11.10 MetaHuman 完整制作流程指南

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、完整制作流程

```
阶段 1：角色创建
  MetaHuman Creator（metahuman.unrealengine.com）
    → 调整外观（脸型/肤色/发型/体型）
    → 导出到 UE 项目（Bridge 同步）

阶段 2：身体动画准备
  方案 A：使用动画商城资产
    Marketplace 上找 UE5 动画包（与 MetaHuman 兼容）
    直接使用，无需重定向
  
  方案 B：动作捕捉
    Rokoko / Xsens / OptiTrack 全身动捕
    → Live Link 实时预览
    → Sequencer 录制
    → 导出 Animation Sequence
  
  方案 C：手 K 动画
    Sequencer + Control Rig
    动画师用控制器 K 帧

阶段 3：面部动画准备
  实时：iPhone（Live Link Face App）+ ARKit
  录制：iPhone 录制 → 导入 UE → Sequencer 编辑
  程序化：预设表情 + 口型同步（见第 11.6 节）

阶段 4：身体 + 面部合并
  Sequencer 中：
    Body Track：身体 Animation Sequence
    Face Track：面部 Animation Sequence（由 ARKit 录制）
    两轨道同时播放 → 完整角色表演

阶段 5：后处理
  曲线平滑（减少动捕抖动）
  细节微调（手工 K 帧修正）
  过渡处理（Inertialization）

阶段 6：导出 / 集成
  Bake 为最终 Animation Sequence
  或保持 Sequencer 格式（过场动画）
  集成到游戏 AnimBP 系统
```

---

## 二、各阶段工具对应

```
阶段            工具                          插件
────────────────────────────────────────────────────────────────
角色创建        MetaHuman Creator             -
身体动捕        Rokoko Smartsuit Pro          Rokoko Live Link
              Xsens MVN                      Xsens Live Link
              iPhone（简单移动）             ARKit Live Link
面部捕捉        iPhone（Live Link Face）      Apple ARKit Face
              Faceware / Dynamixyz           专用插件
Sequencer K帧  UE5 Sequencer + Control Rig   -
后处理          Sequencer 曲线编辑器         -
导出            Bake to Animation Sequence    -
```

---

## 三、制作时间参考

```
独立开发者（1人团队）：
  MetaHuman 创建：~2~4 小时
  重定向现有动画：~0.5~1 天
  iPhone 面部捕捉（1段）：~2~4 小时（含后处理）
  集成进游戏 AnimBP：~1~2 天

小团队（2~3人，专业动画师）：
  完整 NPC（身体+面部+过场）：~3~5 天
  主角完整动画套装：~2~4 周

注意：
  面部后处理是耗时大头（每 1 分钟面部动画后处理 ~30~60 分钟）
  使用 AI 辅助工具（如 NVIDIA Audio2Face）可大幅节省面部动画时间
```

---

## 四、一键口型同步

```
NVIDIA Audio2Face（免费工具）：
  输入：音频文件（对话语音）
  输出：ARKit BlendShape 动画数据（自动口型）
  
工作流：
  录制/配音 → 音频文件
  Audio2Face 生成 ARKit JSON
  导入 UE → 转换为 Animation Sequence
  → Face AnimBP 播放 → MetaHuman 自动对口型

效果：
  基础口型质量：良好（适合 NPC/次要角色）
  精细调整：需要在 Sequencer 中手工修正嘴唇细节
```

---

## 五、延伸阅读

- 🔗 [MetaHuman 完整文档](https://dev.epicgames.com/documentation/en-us/metahuman/metahuman-documentation)
- 🔗 [MetaHuman Creator](https://metahuman.unrealengine.com)
- 🔗 [NVIDIA Audio2Face](https://www.nvidia.com/en-us/omniverse/apps/audio2face/)
- 🔗 [Lyra + MetaHuman 示例](https://dev.epicgames.com/documentation/en-us/unreal-engine/lyra-sample-game-in-unreal-engine)
