# 1.7 物理帧更新机制

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 45 分钟

---

## 一、游戏线程 vs 物理线程

```
UE 运行时的主要线程：

游戏线程（Game Thread）：
  运行游戏逻辑：Tick、蓝图、AI、输入处理
  更新 Actor 变换（从物理结果）
  调用物理 API（AddForce/SetVelocity 等）
  典型帧率：30~120 FPS

渲染线程（Render Thread）：
  准备渲染数据，提交 GPU 命令
  与游戏线程异步（1帧延迟）

物理线程（Physics Thread / Task Graph）：
  运行 Chaos 物理求解
  独立于游戏帧率运行！
  典型物理步长：16.7ms（60Hz物理）

RHI线程（Rendering Hardware Interface）：
  GPU 命令提交

时间线（理想情况，60 FPS / 60 Hz 物理）：
  Frame N：
    [Game Thread: Logic + Push Physics]
    [Physics Thread: Solve Frame N-1]
    [Render Thread: Render Frame N-1]
  Frame N+1：
    [Game Thread: Read Physics N-1 results]
    [Physics Thread: Solve Frame N]
    ...
```

---

## 二、物理帧延迟

```
关键概念：物理结果有 1 帧延迟！

原因：
  游戏线程在帧开始时推送物理命令
  物理线程在当前帧内异步计算
  游戏线程在下一帧开始时读取物理结果

影响：
  玩家施加力 → 下一帧才看到运动
  碰撞检测（LineTrace）→ 基于上一帧的物理状态
  → 在极端情况下可能"看到"物体还在旧位置

实际测量延迟：
  默认设置：约 16.7ms（1 帧 @ 60 FPS）
  启用 AsyncPhysics：可能 1~2 帧额外延迟

可见性问题对应方案：
  视觉表示（Mesh）跟随物理位置（无延迟感知）
  游戏逻辑在 Tick 中基于 Component 位置计算
  → 即使物理延迟 1 帧，视觉上感知不到
  → 游戏逻辑用的是"应用后"的变换，也是延迟的
```

---

## 三、异步物理（Async Physics）

```
UE5 引入 Async Physics Tick：
  物理以固定步长运行，与渲染帧率解耦
  
  Project Settings → Physics:
    Async Physics → Enable Async Physics = true
    Fixed Time Step = 0.016666（60Hz）或 0.033333（30Hz）

优点：
  物理确定性更高（相同输入 → 相同结果）
  物理不受帧率波动影响（60fps / 30fps 物理行为一致）
  网络物理同步更准确（确定性是前提）

缺点：
  额外 1~2 帧延迟（物理缓冲）
  需要插值显示（Component 位置需在物理帧之间插值）

在 Async Physics Tick 中执行逻辑：
  // 实现 AsyncPhysicsTickComponent
  void UMyPhysicsTickComp::AsyncPhysicsTickComponent(
      float DeltaTime, float SimTime)
  {
      // 这里的代码在物理线程上运行（固定步长）
      // 可以安全地调用 FBodyInstance API
      FBodyInstance* BI = MyMesh->GetBodyInstance();
      BI->AddForce(FVector(0, 0, 100.f), false, false);
  }
```

---

## 四、物理同步点

```
游戏线程与物理线程的同步点：

1. StartPhysics（帧开始）：
   游戏线程将本帧命令（力/速度/变换修改）
   推送到 Chaos Command Queue
   → 物理线程开始异步求解

2. EndPhysics（帧结束前）：
   游戏线程等待物理线程完成（如果未完成则 stall）
   从 Physics Proxy 读取求解结果
   → 更新所有 Physics-Simulating Component 的变换
   → 触发 Hit/Overlap 事件回调

同步开销（stall）：
  如果物理计算时间 > 游戏逻辑时间：
    游戏线程在 EndPhysics 处等待
    → 整体帧率降低
  
  Profile 查看：
    stat physics → 找 FPhysScene_Chaos::EndFrame 耗时
    > 2ms 说明物理是瓶颈

强制同步（调试/测试用）：
  p.Chaos.Solver.FixedTimeStep 0   ← 禁用异步，同步执行
  注意：会降低性能，仅调试用
```

---

## 五、物理回调时机

```
物理事件在游戏线程的回调时机：

OnComponentHit（碰撞 Hit）：
  触发时机：EndPhysics 之后，同帧游戏 Tick 之前
  线程：游戏线程（安全）
  
OnComponentBeginOverlap / EndOverlap：
  同上

OnComponentWake / Sleep：
  同上

注意事项：
  在 OnComponentHit 回调中不能调用某些 Chaos API！
  （物理正在被读取，不能修改）
  → 如需修改物理状态（施加力/修改速度），
    用 FTimerManager::SetTimerForNextTick 推迟到下帧

PhysicsReplication（物理复制，多人游戏）：
  服务器物理状态 → 通过网络发送 → 客户端接收
  客户端在 EndPhysics 之前应用服务器状态修正
  详见第十二章：物理网络同步
```

---

## 六、延伸阅读

- 📄 [1.8 子步进（Substepping）](./08-substepping.md)
- 🔗 [Async Physics Tick](https://dev.epicgames.com/documentation/en-us/unreal-engine/async-physics-in-unreal-engine)
