# 6.10 Motion Matching 性能优化

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 20 分钟

---

## 一、MM 搜索性能

```
Motion Matching 的核心开销：姿势搜索（Pose Search）

每次搜索：
  遍历数据库中所有帧（或使用 KD-Tree 加速）
  计算每帧的匹配 Cost
  找到最小 Cost 的帧

开销估算：
  数据库 50 帧 × 5 特征：  ~0.02ms/角色
  数据库 200 帧 × 10 特征：~0.1ms/角色
  数据库 1000 帧 × 20 特征：~0.5ms/角色（需要优化）

100 个角色的总开销（200帧/10特征）：
  ~10ms（可能占用帧时间的 1/6）→ 需要优化
```

---

## 二、降低搜索频率

```
不需要每帧都搜索新帧：

配置：
  Motion Matching 节点 → Min Time Between Pose Reselects: 0.1s
  → 每 0.1 秒搜索一次（10 FPS 搜索频率）
  → 30~60 FPS 的帧不做搜索，直接继续播放当前动画
  
  效果：搜索开销减少 3~6 倍
  
  注意：间隔过长会降低响应性
    推荐范围：0.05s ~ 0.2s
```

---

## 三、数据库剪枝（Pruning）

```
减少数据库有效帧数：

策略 1：标记排除区域（Exclusion Zones）
  将过渡帧标记为 Blocked → 减少搜索候选帧数

策略 2：按标签过滤（Tags Filter）
  只搜索当前状态相关的动画子集
  战斗状态 → 只搜索 Combat 标签的动画
  
策略 3：数据库分层
  高频动作（Walk/Run）放小数据库（精搜）
  低频动作（急转/急停）放大数据库（粗搜）
```

---

## 四、LOD 控制

```
远距离角色降低搜索精度：

LOD 0（<15m）：  正常搜索，MinReselect = 0.05s
LOD 1（15~40m）：MinReselect = 0.15s（降低频率）
LOD 2（>40m）：  禁用 MM，使用简化状态机
                或停止更新动画（Tick Throttle）

配置：
  Motion Matching 节点 → LOD Threshold: 1（LOD2+ 禁用）
  + Animation Budget Allocator（见第一章优化）控制更新频率
```

---

## 五、KD-Tree 加速

```
UE 的 Pose Search 使用 PCA 降维 + KD-Tree 加速最近邻搜索：

默认已启用，无需手动配置
但可以调整：

Pose Search Database → Advanced:
  PCA Components: 4（降维后的维数，越小越快但精度下降）
  
建议保持默认（6~8），大数据库（1000+ 帧）可降至 4
```

---

## 六、优化清单

```
□ Min Time Between Pose Reselects >= 0.05s
□ 使用 Tags 过滤当前场景的数据库子集
□ 标记所有不适合作为起点的帧（Exclusion）
□ LOD2+ 禁用 MM 节点
□ Animation Budget Allocator 限制远处角色更新频率
□ 监控 Insights: PoseSearch.Select 耗时
□ 数据库大小控制在 500 帧以内（每个数据库）
```

---

## 七、延伸阅读

- 🔗 [Pose Search 性能说明](https://dev.epicgames.com/documentation/en-us/unreal-engine/pose-search-in-unreal-engine)
- 🔗 [Animation Budget Allocator](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-budget-allocator-in-unreal-engine)
