# 2.10 骨骼系统性能优化

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、性能分析工具

```
骨骼动画性能分析工具：

1. Unreal Insights（推荐）
   运行游戏 → 连接 Unreal Insights → 查看 Animation 轨道
   关注：AnimTask（Worker Thread 动画评估时间）

2. stat anim（控制台命令）
   显示：每帧动画 CPU 时间、骨骼数量、IK 次数

3. Gameplay Debugger（' 键启用）
   可视化：当前 LOD、动画更新频率

4. GPU 分析
   RenderDoc / PIX：分析 GPU Skinning 耗时
   关注：Vertex Buffer Update、Skinning Pass
```

---

## 二、URO（Update Rate Optimization）

URO 是 UE 内置的动画更新率优化系统，**对性能影响极大**：

```cpp
// 在 SkeletalMeshComponent 上配置 URO
USkeletalMeshComponent* Mesh = GetMesh();

// 开启 URO
Mesh->EnableUpdateRateOptimizations();

// 配置 URO 参数
FAnimUpdateRateParameters* URO = Mesh->AnimUpdateRateParams;
if (URO)
{
    // 根据屏幕占比降低更新频率：
    // 屏幕占比 > 0.25：每帧更新（正常）
    // 屏幕占比 0.1-0.25：每2帧更新
    // 屏幕占比 < 0.1：每3帧更新
    // 不可见：完全停止更新

    URO->BaseNonRenderedUpdateRate = 4;   // 不渲染时更新频率（每N帧1次）
    URO->bShouldUseLodMap = true;         // 根据 LOD 调整更新率
    URO->LODToFrameSkipMap.Add(0, 0);    // LOD0：不跳帧
    URO->LODToFrameSkipMap.Add(1, 1);    // LOD1：每2帧更新1次
    URO->LODToFrameSkipMap.Add(2, 3);    // LOD2：每4帧更新1次
    URO->LODToFrameSkipMap.Add(3, 7);    // LOD3：每8帧更新1次
}
```

---

## 三、可见性驱动的动画更新

```cpp
// 配置：仅在渲染时更新动画姿势（最重要的单项优化）
Mesh->VisibilityBasedAnimTickOption =
    EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
// 效果：屏幕外的角色不做任何动画计算
// 注意：如果角色对游戏逻辑有影响（如脚步声、触发器），
//       需要改为 AlwaysTickPoseAndRefreshBones

// 其他可选值：
// AlwaysTickPoseAndRefreshBones：永远更新（最贵，用于重要角色）
// AlwaysTickPose：更新姿势但不刷新骨骼（节省 FK 计算）
// OnlyTickMontagesWhenNotRendered：只处理 Montage（轻量）
// OnlyTickPoseWhenRendered：最省，推荐 NPC/背景角色
```

---

## 四、LOD 骨骼裁减

```
在 Skeletal Mesh Editor → LOD Settings 中配置：

LOD0（近距离，全质量）：
  骨骼数：100%（所有骨骼）
  Cloth：✅ 完整模拟
  IK：✅ 完整 IK

LOD1（中距离）：
  裁减骨骼：手指骨骼（finger_XX），面部骨骼
  减少到约 70% 骨骼
  Cloth：✅ 简化模拟

LOD2（远距离）：
  仅保留主干骨骼（root, pelvis, spine, clavicle, upper/lower arm/leg, head）
  减少到约 30% 骨骼
  Cloth：❌ 禁用
  IK：❌ 禁用（在 AnimBP 中根据 LOD 级别禁用 IK 节点）

LOD3（极远距离）：
  最少骨骼（仅 root + pelvis + 主肢体）
  单一循环动画，不用 AnimBP
```

---

## 五、AnimBP 优化

```
AnimBP 常见性能问题：

1. 每帧大量 Cast 操作
   ❌ 坏: (AMyCharacter*)TryGetPawnOwner()  （每帧 Cast）
   ✅ 好: 在 NativeInitializeAnimation() 中缓存 Cast 结果

2. 在 AnimBP Event Graph 中做复杂计算
   ❌ 坏: Event Graph 大量蓝图节点
   ✅ 好: 迁移到 C++ NativeThreadSafeUpdateAnimation

3. 不必要的 IK 计算
   ❌ 坏: 远处 LOD2 角色仍运行完整 FBIK
   ✅ 好: 用 LOD Threshold 限制 IK 节点只在 LOD0/1 运行
   
   在 AnimGraph 中的 IK 节点 → Details → LOD Threshold: 1
   （LOD > 1 时自动 pass through，不执行 IK）

4. 过多的 Blend 节点
   建议：减少不必要的混合层数，合并相似条件的 State Machine
```

---

## 六、并发优化

```
UE5 并发动画配置：

// project.ini 中（或项目设置 → Engine → Animation）
[/Script/Engine.AnimationSettings]
bTickAnimationOnSkeletalMeshInit=false
bUseMultiThreadedAnimationUpdate=true    // 启用多线程（默认 true）
bEnableAnimationThreadedEvaluation=true  // 启用线程化评估

线程模型：
  Game Thread:    NativeUpdateAnimation（读取 Gameplay 状态）
  Worker Thread:  AnimGraph 评估（骨骼计算）
  Render Thread:  GPU Skinning（顶点变形）

线程安全要点：
  • 不要在 AnimNode 中访问 Actor/Component（非线程安全）
  • 使用 Property Access 系统替代直接访问
  • Worker Thread 只能操作 FTransform、FVector 等值类型
```

---

## 七、性能优化效果参考

```
优化措施与预期收益（100个NPC场景）：

措施                                  CPU 节省    实现难度
──────────────────────────────────────────────────────
开启 URO                              30-50%      低
OnlyTickPoseWhenRendered             20-40%      低（一行代码）
LOD 骨骼裁减（LOD1+）                 15-25%      中
AnimBP 变量 Cast 缓存                10-20%      低
禁用远处 IK                           10-15%      低（LOD Threshold）
Master Pose Component（多部件角色）   10-30%      中
迁移逻辑到 C++ ThreadSafe 函数        5-15%       中
```

---

## 八、延伸阅读

- 🔗 [UE5 动画性能优化（知乎）](https://zhuanlan.zhihu.com/p/1927668362969880273)
- 🔗 [《重装前哨》海量怪物优化案例](https://zhuanlan.zhihu.com/p/693413755)
- 🔗 [Epic 官方性能指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/performance-guidelines-for-mobile-devices-in-unreal-engine)
- 🔗 [Animation Optimization 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine)
