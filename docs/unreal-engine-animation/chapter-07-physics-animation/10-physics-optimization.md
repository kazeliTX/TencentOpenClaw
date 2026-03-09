# 7.10 物理动画性能优化

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 20 分钟

---

## 一、各类物理动画开销

```
（以单角色为参考基准）

技术                        开销       线程
────────────────────────────────────────────────────
布娃娃（20 Bodies）         1~3ms      Game Thread
Physical Animation Component ~0.5ms    Game Thread
Rigid Body AnimGraph 节点   0.2~0.5ms  Worker Thread
AnimDynamics（3骨骼链）     ~0.05ms    Worker Thread
Spring Bone                 ~0.01ms    Worker Thread
Chaos Cloth（500顶点）      0.5~2ms    Worker Thread
Groom 模拟（5000发丝）      2~10ms     Game Thread（Niagara）
```

---

## 二、布娃娃优化

```
1. 减少 Body 数量
   手指合并 → 手部用 1 个 Body（而非 15 个）
   脸部骨骼不需要 Body
   目标：< 20 个 Bodies / 角色

2. 最大同时布娃娃数量限制
   场景中同时超过 5~8 个活跃布娃娃开始明显影响帧率
   实现：超出时销毁最旧的布娃娃或切换为静止网格
   
3. 布娃娃 LOD
   距离 > 15m：降低 Body 数量（使用简化 Physics Asset）
   距离 > 30m：切换为静止 Pose（禁用物理，保留最终姿势）
   
4. 死亡后延迟销毁
   布娃娃静止后（速度 < 5cm/s），等待 3~5s 后销毁
   或直接 SetSimulatePhysics(false) + 保持最终 Pose
```

---

## 三、布料与发丝优化

```
Chaos Cloth：
  每角色布料顶点 < 300 个（高性能要求平台）
  LOD2+ 禁用布料模拟（显示静止贴图）
  减少 Solver 迭代：SubStep 1（默认 2）

Groom 发丝：
  PC/主机 LOD0：500~2000 发丝
  移动平台：禁用 Groom，使用 Cards 或 Mesh 发型
  距离 > 10m：自动切换为 Cards

命令：
  r.HairStrands.Enable 0   ← 禁用发丝（移动设备建议全局禁用）
  r.ClothSimulation 0      ← 禁用布料模拟（调试用）
```

---

## 四、Physical Animation 优化

```
Physical Animation Component：
  只在需要时激活（死亡/受击），不需要时关闭
  ApplyPhysicsProfile → 不需要物理时调用 None profile
  
AnimDynamics / Spring Bone：
  这两类节点已经在 Worker Thread 运行，开销低
  LOD1+ 可以简化（减少链长度）
  LOD2+ 禁用节点（LOD Threshold）
```

---

## 五、性能监控命令

```
stat physics        ← 物理总开销
stat physicsx       ← PhysX/Chaos 详细分解
p.VisualizePhysics  ← 可视化物理体

Insights Profiler：
  搜索 Chaos.Tick        ← Chaos 物理总时间
  搜索 AnimInstance.Tick ← 动画（含 AnimDynamics/RigidBody）时间
  搜索 Cloth.Tick        ← 布料模拟时间
```

---

## 六、延伸阅读

- 🔗 [Physics Performance 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-best-practices-in-unreal-engine)
- 🔗 [Chaos Cloth 性能指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/clothing-tool-in-unreal-engine)
