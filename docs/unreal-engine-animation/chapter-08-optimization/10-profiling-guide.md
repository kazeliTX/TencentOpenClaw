# 8.10 动画性能分析实战指南

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、定位动画性能问题的步骤

```
Step 1：确认瓶颈在动画系统
  stat unit → 查看 Game/Draw/GPU 各线程时间
  如果 Game 时间高 → 可能是动画（或 AI、物理）
  
  stat anim → 直接看动画开销
    AnimGameThreadTime > 3ms → 需要优化
    
Step 2：找到最慢的 AnimInstance
  Insights → 启动 Trace
  在游戏中触发性能问题场景
  停止 Trace → 用 Insights 分析
  搜索：AnimInstance 按耗时排序
  找到耗时最长的几个角色
  
Step 3：分析 AnimGraph
  选中目标 AnimInstance → 查看哪个节点最慢
  常见热点：FABRIK（多骨骼链）、Full Body IK、Rigid Body
  
Step 4：应用优化，对比数据
```

---

## 二、实用控制台命令

```bash
# 动画相关
stat anim                     # 动画总开销
stat animverbose              # 详细动画统计
stat skeletalmesh             # 骨骼 Mesh 统计
stat physics                  # 物理开销（含布娃娃）

# 可视化
ShowFlag.Bones 1              # 显示所有骨骼
a.ShowPose 1                  # 显示动画姿势信息
a.URO.EnableDebug 1           # 显示每个角色的动画更新频率（URO 调试）
a.AnimBudget.Debug 1          # 显示 Budget Allocator 状态

# 强制 LOD（测试用）
a.ForceLOD 0                  # 强制所有骨骼 Mesh 使用 LOD0
a.ForceLOD 2                  # 强制 LOD2（测试优化效果）
a.ForceLOD -1                 # 恢复自动 LOD

# 禁用某些功能（隔离测试）
a.ParallelAnimEvaluation 0    # 禁用并行动画求值（调试用）
p.Cloth.Enable 0              # 禁用布料
r.HairStrands.Enable 0        # 禁用发丝
```

---

## 三、性能优化工作流（实战 Checklist）

```
□ 第一步：基础设置
  □ OnlyTickPoseWhenRendered = true（所有 NPC）
  □ 启用 Animation Budget Allocator
  □ 确认 LOD 距离阈值合理

□ 第二步：AnimBP 快速路径
  □ 编译 AnimBP，清除所有 Fast Path 警告
  □ 将所有 AnimGraph 输入移到 NativeUpdateAnimation
  □ 复杂计算移到 NativeThreadSafeUpdateAnimation

□ 第三步：节点 LOD 阈值
  □ IK 节点：LOD Threshold = 1
  □ Aim Offset：LOD Threshold = 1
  □ 物理节点（AnimDynamics/Rigid Body）：LOD Threshold = 1
  □ FABRIK：LOD Threshold = 1

□ 第四步：LOD 专用简化 AnimGraph
  □ LOD2 使用简化状态机（3~5 状态）
  □ LOD3 使用单一动画序列

□ 第五步：大规模场景
  □ 相同 NPC 使用 AnimToTexture + ISM
  □ 远处角色（>50m）切换为 VAT 方案
  □ 使用 Niagara 方案处理 500+ 人群
```

---

## 四、常见性能问题与解决方案

| 问题 | 症状 | 解决方案 |
|------|------|---------|
| AnimInstance 太慢 | stat anim 高 | Fast Path + Thread Safe |
| 太多同时活跃骨骼动画 | 帧率在人多时骤降 | Budget Allocator + LOD |
| 屏幕外角色仍在计算 | 关掉摄像机后 CPU 不降低 | OnlyTickPoseWhenRendered |
| IK 开销过高 | stat skeletal controls 高 | LOD Threshold + 降低检测频率 |
| 布娃娃导致卡顿 | 死亡时帧率骤降 | 限制最大布娃娃数量 |
| 大量相同 NPC | 人群场景卡顿 | AnimToTexture + ISM |

---

## 五、延伸阅读

- 🔗 [Animation Optimization 完整指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine)
- 🔗 [Unreal Insights 使用指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine)
- 🔗 [Animation Budget Allocator](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-budget-allocator-in-unreal-engine)
