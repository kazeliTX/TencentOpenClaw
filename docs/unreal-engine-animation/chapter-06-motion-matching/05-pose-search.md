# 6.5 Pose Search（姿势搜索）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、Pose Search Schema（特征模式）

Schema 定义了用哪些数据描述一个动画帧：

```
创建 Pose Search Schema：
  内容浏览器 → 右键 → Animation → Pose Search → Pose Search Schema
  命名：PSS_Locomotion

Schema 配置（Channel 列表）：

Channel 1：姿势骨骼（Pose Bone Positions）
  Type: Pose
  Bones:
    • root（根骨骼，描述整体位移趋势）
    • foot_l（左脚，描述步态相位）
    • foot_r（右脚，描述步态相位）
  Features: Position（位置）+ Velocity（速度）
  Weight: 1.0
  
Channel 2：过去轨迹（Past Trajectory）
  Type: Trajectory Positions（或 Trajectory Times）
  Sample Times: -0.2, -0.1（过去 0.1s 和 0.2s 的轨迹点）
  Features: Position + Facing Direction
  Weight: 0.8

Channel 3：未来轨迹（Future Trajectory）  
  Type: Trajectory Positions
  Sample Times: 0.1, 0.2, 0.3（未来 0.1~0.3s 的预测轨迹）
  Features: Position + Facing Direction
  Weight: 1.5（未来轨迹权重更高，确保响应输入）
```

---

## 二、Pose Search Database（姿势数据库）

```
创建 Pose Search Database：
  内容浏览器 → 右键 → Animation → Pose Search → Pose Search Database
  命名：PSD_Locomotion
  
  Schema: PSS_Locomotion（关联上面的 Schema）

添加动画到数据库：
  Animations 列表 → + 添加：
  
  序号  动画资产               标签（Tags）
  ────────────────────────────────────────
  1     A_Idle                Idle
  2     A_Walk_Fwd            Walk
  3     A_Walk_Left           Walk, Strafe
  4     A_Walk_Right          Walk, Strafe
  5     A_Walk_Bwd            Walk
  6     A_Run_Fwd             Run
  7     A_Run_Left            Run, Strafe
  8     A_Run_Right           Run, Strafe
  9     A_Run_Bwd             Run
  10    A_Jog_Start_Fwd       Start
  11    A_Jog_Stop            Stop
  12    A_Turn_Left_90        Turn
  13    A_Turn_Right_90       Turn
  ... （建议 20~50+ 个动画）

注意：
  所有动画必须用同一个 Skeleton
  动画质量越高，Motion Matching 效果越好
  推荐使用动作捕捉数据（而非手K）
```

---

## 三、特征归一化（Normalization）

```
不同特征的量纲不同（位置单位 cm，速度单位 cm/s）
直接比较会导致权重失衡

UE Pose Search 自动处理归一化：
  Schema → Advanced → Normalize Features: ✅
  → 自动将所有特征缩放到 [-1, 1] 范围

手动调整权重：
  对于不同场景，调整各 Channel 的 Weight：
  
  强调响应性（格斗游戏）：
    Future Trajectory Weight: 2.0~3.0（未来轨迹权重高）
    Pose Weight: 0.5（允许姿势有更大跳变）
  
  强调自然过渡（RPG）：
    Future Trajectory Weight: 1.0
    Pose Weight: 1.5（保持姿势连续性）
```

---

## 四、数据库预处理

```
在编辑器中：Pose Search Database → Build（构建数据库）

构建过程：
  1. 遍历所有动画的所有帧
  2. 为每帧提取特征向量（按 Schema 定义）
  3. 存储到 KD-Tree 或类似数据结构（加速搜索）
  4. 完成后可在 Pose Search Database 视图中预览

构建时间：
  50 个动画，Schema 5 个特征：约 10~30 秒
  200 个动画：约 1~3 分钟
  
构建后：数据库资产变大（每帧一个特征向量）
建议：仅在需要时重新构建，不要每次打包都构建
```

---

## 五、延伸阅读

- 📄 [6.7 Motion Matching 数据库构建](./07-mm-database.md)
- 🔗 [Pose Search Plugin 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/pose-search-in-unreal-engine)
