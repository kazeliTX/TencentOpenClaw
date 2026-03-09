# 6.7 Motion Matching 数据库构建

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、动画录制要求

```
Motion Matching 动画质量要求（高于传统状态机）：

必要条件：
  1. 所有动画使用相同的 Skeleton
  2. 动画需要"干净"的真实运动（推荐 Mocap）
  3. 根骨骼包含完整的位移数据
  4. 动画长度足够（循环动画 >= 2 个周期）

推荐最小动画集（~30~50 个）：
  Idle × 1
  Walk × 8 方向
  Run  × 8 方向
  Start（起步）× 4~8
  Stop（停止） × 4~8
  Turn in Place × 4
```

---

## 二、数据库分组（Tags）

```
通过标签过滤不同场景的动画子集：

Group 1: Locomotion_Ground   → Walk, Run, Idle, Start, Stop
Group 2: Locomotion_Combat   → 持枪行走动画集
Group 3: Locomotion_Crouch   → 蹲姿动画集

AnimGraph 中通过 Database Tags Filter 按需切换
```

---

## 三、Exclusion Zones（排除区域）

```
某些动画帧不适合作为搜索起点，标记为 Blocked：

典型使用：
  急停动画第一帧（滑步）→ 排除前 0.15s
  着陆冲击帧 → 排除前 0.1s

配置位置：
  Pose Search Database → 选中动画 → Excluded Intervals
```

---

## 四、数据库调试

```
运行时调试：
  a.PoseSearch.Debug 1（控制台命令）
  → 绿色：当前播放帧
  → 蓝色：候选帧
  → 显示各帧的匹配 Cost 分布
```

---

## 五、延伸阅读

- 📄 [代码示例：MM 数据库配置](./code/03_motion_matching_config.cpp)
- 🔗 [Pose Search Database 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/pose-search-in-unreal-engine)
