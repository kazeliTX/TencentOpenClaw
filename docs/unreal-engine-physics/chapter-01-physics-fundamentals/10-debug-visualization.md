# 1.10 物理调试可视化

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、视口碰撞显示

```
视口快捷按钮：
  Show → Collision → Simple/Complex Collision
  
  颜色含义：
    绿色碰撞体  = Simulate Physics（动态物理体）
    蓝色碰撞体  = Static（静态阻挡）
    黄色碰撞体  = Query Only（仅射线检测，不参与物理）
    橙色碰撞体  = Physics Only（物理模拟，不参与射线）

  命令行等价：
    show collision         ← 开启碰撞显示
    show collisionvisibility ← 仅显示 Visibility 通道

  碰撞形状类型颜色区分：
    球形碰撞    = 圆形线框
    胶囊碰撞   = 胶囊线框
    盒形碰撞    = 方形线框
    凸包碰撞   = 多边形线框（绿色/青色）
    复杂碰撞   = 三角网格（青色网格，开销大）
```

---

## 二、Chaos Debug Draw 命令

```
p.Chaos.DebugDraw 系列命令（运行时输入）：

p.Chaos.DebugDraw.Enabled 1               ← 启用 Chaos 调试绘制（总开关）

碰撞相关：
  p.Chaos.DebugDraw.ShowCollision 1        ← 显示碰撞形状
  p.Chaos.DebugDraw.ShowContacts 1         ← 显示接触点（红点）
  p.Chaos.DebugDraw.ShowContactNormals 1   ← 显示接触法线（蓝色箭头）
  p.Chaos.DebugDraw.ShowContactPhis 1      ← 显示碰撞穿透深度

运动状态：
  p.Chaos.DebugDraw.ShowVelocity 1         ← 显示速度向量（黄色箭头）
  p.Chaos.DebugDraw.ShowAngularVelocity 1  ← 显示角速度（蓝色弧线）
  p.Chaos.DebugDraw.ShowCenterOfMass 1     ← 显示质心（红色十字）
  p.Chaos.DebugDraw.ShowInertiaTensor 1    ← 显示惯性张量主轴

Island 与 Sleep：
  p.Chaos.DebugDraw.ShowIslands 1          ← 用不同颜色显示 Island
  p.Chaos.DebugDraw.ShowSleeping 1         ← 休眠物体变红色
  p.Chaos.DebugDraw.ShowAwake 1            ← 活跃物体变绿色

约束：
  p.Chaos.DebugDraw.ShowConstraints 1      ← 显示所有约束连线
  p.Chaos.DebugDraw.ShowJoints 1           ← 显示关节约束
  p.Chaos.DebugDraw.ShowSuspension 1       ← 显示悬挂约束（车辆）

BroadPhase：
  p.Chaos.DebugDraw.ShowBroadphase 1       ← 显示 AABB 树
```

---

## 三、性能统计命令

```
stat physics：
  显示物理相关 CPU 统计
  
  关键指标：
    Physics Total：           总物理时间
    Physics BroadPhase：      宽相检测时间
    Physics NarrowPhase：     窄相检测时间
    Physics Solve：           约束求解时间
    Physics Integration：     位置积分时间
    
    Active Rigid Bodies：     活跃刚体数量
    Sleeping Rigid Bodies：   休眠刚体数量
    Collision Pairs：         碰撞对数量

stat chaos：
  Chaos 专项统计
  
  Chaos_Advance：             每帧物理推进时间
  Chaos_BroadPhase：
  Chaos_NarrowPhase：
  Chaos_Solver：

stat game → PhysicsTotal：    游戏线程等待物理的时间（stall）

解读示例：
  Physics Total: 15ms
    BroadPhase: 2ms     ← AABB 更新慢，物体太多
    NarrowPhase: 8ms    ← 窄相检测慢，复杂碰撞形状太多
    Solve: 5ms          ← 约束求解慢，约束太多或迭代次数太高
```

---

## 四、Chaos Visual Debugger（CVD）

```
CVD = 专业的 Chaos 物理调试工具（UE5.2+）

启用：
  Project Settings → Physics → Chaos Visual Debugger:
    Enable Visual Debugger = true
    Debug Output Directory: [项目]/Saved/CVD/

功能：
  录制物理状态（每帧）
  → 在 CVD Viewer 中逐帧回放
  → 可暂停/前进/后退物理帧
  → 可查看任意帧的碰撞/约束/速度状态

CVD Viewer 启动：
  引擎安装目录/Binaries/Win64/ChaosSolverDebugger.exe
  打开录制的 .CVD 文件

CVD 使用场景：
  复现难以重现的物理 Bug
  分析物理崩溃（爆炸/穿透）发生的瞬间
  优化碰撞对数量（查看哪些对象产生过多碰撞）
```

---

## 五、常用调试蓝图节点

```
Draw Debug 系列（蓝图/C++ 均可）：

DrawDebugLine：           绘制调试线
DrawDebugSphere：         绘制调试球
DrawDebugBox：            绘制调试盒
DrawDebugPoint：          绘制调试点
DrawDebugArrow：          绘制调试箭头

物理专用调试：
// C++ 示例：绘制速度向量
FVector Vel = Comp->GetPhysicsLinearVelocity();
DrawDebugArrow(GetWorld(),
    GetActorLocation(),
    GetActorLocation() + Vel * 0.1f,
    10.f, FColor::Yellow, false, -1.f, 0, 2.f);

// 绘制碰撞查询结果
TArray<FHitResult> Hits;
GetWorld()->SweepMultiByChannel(Hits, Start, End, ...);
for (auto& Hit : Hits)
    DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.f,
        FColor::Red, false, 3.f);
```

---

## 六、延伸阅读

- 📄 [代码示例：物理查询](./code/02_physics_query.cpp)
- 🔗 [Chaos Visual Debugger](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-visual-debugger-in-unreal-engine)
- 🔗 [Collision Visualization](https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-in-unreal-engine)
