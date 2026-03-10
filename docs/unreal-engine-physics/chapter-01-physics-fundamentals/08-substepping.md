# 1.8 子步进（Substepping）

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 35 分钟

---

## 一、为什么需要子步进

```
问题：帧率降低时物理行为改变

案例：弹簧约束（Spring）
  60 FPS：步长 Δt = 16.7ms → 弹簧稳定振荡
  15 FPS：步长 Δt = 66.7ms → 弹簧爆炸发散！
  
  原因：数值积分稳定性依赖步长
    显式欧拉积分：稳定条件 Δt < 2/ω（ω=弹簧角频率）
    步长越大，越容易发散

另一个问题：持续力的帧率依赖
  在 Tick 中每帧施加力：AddForce(F)
  60 FPS：力作用 16.7ms/帧，稳定加速
  30 FPS：力作用 33.3ms/帧，每帧力效果翻倍
  → 物理行为帧率依赖！

子步进（Substepping）解决方案：
  将一个游戏帧分成多个小物理步
  每个小步用更小的 Δt
  → 无论游戏帧率如何，物理步长固定 → 行为一致
```

---

## 二、子步进配置

```
Project Settings → Physics → Substepping:

Enable Substepping:          true（开启子步进）
Max Substep Delta Time:      0.016666（每子步最大时长，≈60Hz）
Max Substeps:                6（每帧最多 6 个子步）

工作原理：
  游戏帧 Δt = 0.1s（10 FPS 极端情况）
  Max Substep Δt = 0.016666s
  子步数 = ceil(0.1 / 0.016666) = 6（最多 6 步）
  
  如果 Δt/MaxSubstepDt > MaxSubsteps：
    使用 MaxSubsteps 个子步，每步 Δt/MaxSubsteps
    → 时间拉伸（慢动作），但不崩溃

性能影响：
  每个子步都要完整执行物理求解
  子步 6 个 = 物理时间 6x
  一般游戏：2~4 子步足够
  
  建议配置（平衡精度与性能）：
    Max Substep Δt = 0.016666（60Hz）
    Max Substeps = 4
    → 最低支持 15 FPS 稳定物理
```

---

## 三、子步进回调

```
在子步进中执行代码（每个子步调用一次）：

重要：普通 Tick 函数每帧只调用一次！
     在 Tick 中施加力 → 帧率依赖
     在子步进回调中施加力 → 帧率无关 ✅

注册子步进回调（C++）：
  // 在 BeginPlay 中注册
  FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();
  OnPhysicsSubstepHandle = PhysScene->OnPhysicsSubstep.AddUObject(
      this, &AMyActor::PhysicsSubstep);

  // 子步进回调函数
  void AMyActor::PhysicsSubstep(float DeltaTime, FBodyInstance* BodyInst)
  {
      // DeltaTime = 子步进步长（固定值，如 0.016666）
      // BodyInst  = 该物体的物理实例
      
      // 安全施加力（子步进内）
      FVector ThrustForce = GetActorForwardVector() * 50000.f;
      BodyInst->AddForce(ThrustForce, false, false);
  }

  // 在 EndPlay 中注销
  GetWorld()->GetPhysicsScene()->OnPhysicsSubstep.Remove(OnPhysicsSubstepHandle);

注意：子步进回调在物理线程运行！
  → 不能调用游戏线程 API
  → 不能读取 Actor 位置（需要先在游戏线程存到成员变量）
  → 只能调用 FBodyInstance 的线程安全方法
```

---

## 四、固定物理帧率 vs 可变帧率

```
方案比较：

方案 A：可变步长（默认）
  物理步长 = 游戏帧 Δt
  简单，低延迟
  缺点：帧率变化导致物理不确定
  适用：单人游戏，不需要网络同步

方案 B：子步进（Substepping）
  物理步长固定（如 16.7ms）
  游戏帧被分成多个物理步
  缺点：帧率低时 CPU 开销增加
  适用：需要稳定物理质量的游戏

方案 C：异步物理固定帧率（推荐，UE5）
  Project Settings → Async Physics → Enable = true
  Fixed Time Step = 0.016666
  物理以独立固定帧率运行
  游戏线程插值显示中间帧
  优点：最高确定性，最适合网络同步
  缺点：视觉延迟需要插值处理
  适用：多人物理游戏，竞技类

网络同步的推荐设置（第十二章预告）：
  Async Physics + 固定步长 + 确定性种子
  → 相同客户端输入 → 完全相同的物理结果
  → 网络预测/回滚更准确
```

---

## 五、调试子步进

```
查看实际子步进情况：
  stat physics → SubstepCount（每帧平均子步数）

调试命令：
  p.SubsteepingAsync 1        ← 查看异步子步进状态
  p.MaxPhysicsDeltaTime 0.1   ← 设置物理最大步长（秒）

常见问题：
  Q：开了子步进但物理还是不稳定
  A：检查 Max Substep Δt 是否足够小（建议 ≤ 0.0166）
     检查弹簧刚度是否过高（刚度过高需要更小步长）

  Q：子步进导致 CPU 飙升
  A：减少 Max Substeps（4→2）
     或提高 Max Substep Δt（放宽精度要求）
     或减少参与物理的对象数量
```

---

## 六、延伸阅读

- 📄 [代码示例：子步进回调](./code/03_substepping_demo.cpp)
- 📄 [1.9 物理场景与世界设置](./09-physics-scene.md)
- 🔗 [Physics Substepping](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-sub-stepping-in-unreal-engine)
