# 1.2 Chaos 引擎内部架构

> **难度**：⭐⭐⭐⭐⭐ | **阅读时间**：约 45 分钟

---

## 一、Chaos 总体架构

```
Chaos 物理引擎分层架构：

┌─────────────────────────────────────────────────────────┐
│                   Engine Layer（引擎层）                  │
│  UPrimitiveComponent / UPhysicsConstraintComponent       │
│  APhysicsVolume / UDestructibleComponent                 │
└────────────────────┬────────────────────────────────────┘
                     │ FPhysicsActorHandle / FConstraintHandle
┌────────────────────▼────────────────────────────────────┐
│               Physics Interface Layer（接口层）           │
│  IPhysicsInterface_Chaos                                 │
│  FPhysScene_Chaos（每个 World 一个）                     │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│              Chaos Solver Layer（求解器层）               │
│  Chaos::FPhysicsSolver                                   │
│  ├── FPBDRigidsSolver（刚体 PBD 求解）                   │
│  ├── FPBDJointConstraintSolver（关节约束）               │
│  ├── FPBDCollisionConstraintSolver（碰撞约束）           │
│  └── FPBDSuspensionConstraintSolver（悬挂约束）         │
└────────────────────┬────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────┐
│              Chaos Core Layer（核心层）                   │
│  Chaos::FParticles（粒子系统，刚体/布料/流体共享）       │
│  Chaos::FImplicitObject（碰撞几何体）                    │
│  Chaos::FContactConstraint（接触约束）                   │
└─────────────────────────────────────────────────────────┘
```

---

## 二、Physics Proxy 机制

```
Physics Proxy 是游戏线程与物理线程之间的桥梁：

游戏线程                        物理线程
────────────────────────────────────────────────────────
UPrimitiveComponent             Chaos::FPhysicsSolver
       │                               │
       ▼                               ▼
FSingleParticlePhysicsProxy ←──── Chaos::FRigidBodyParticle
  （Game Thread 侧代理）          （Physics Thread 侧粒子）

数据同步机制：
  Push（游戏 → 物理）：
    Component 变换/速度/力 → 写入 Proxy Command Queue
    物理线程在下一个 AdvanceOneTimeStep 时消费命令

  Pull（物理 → 游戏）：
    物理求解完成 → 结果写入 Proxy Result Buffer
    游戏线程在 PhysicsEndCallback 时读取结果
    → 更新 Component 的 WorldTransform

双缓冲设计：
  防止读写竞争
  Write Buffer：物理线程写入当前帧结果
  Read Buffer：游戏线程读取上一帧结果
  每帧 Swap 两个 Buffer
```

---

## 三、PBD（Position Based Dynamics）求解器

```
Chaos 使用 PBD（基于位置的动力学）而非传统 impulse-based：

传统方法（PhysX/早期引擎）：
  1. 施加力 → 计算加速度 → 更新速度 → 更新位置
  2. 检测穿透 → 施加冲量 → 再次更新速度/位置
  问题：多次迭代才能收敛，高速物体容易穿透

PBD 方法（Chaos）：
  1. 预测位置（忽略约束）
  2. 通过约束投影直接修正位置（不经过速度）
  3. 从位置变化反推速度
  
  优点：
    稳定性好（不会爆炸式发散）
    约束收敛快
    布料/软体性能好
  缺点：
    能量不完全守恒（阻尼特性）
    对 Restitution（弹性）的模拟精度略低于 impulse-based

XPBD（Extended PBD）—— Chaos 5.x 使用：
  在 PBD 基础上引入约束刚度（Compliance）参数
  更物理正确的弹簧/布料模拟
  公式：Δλ = -(C(x) + α̃λ) / (∇C^T M^-1 ∇C + α̃)
  其中 α̃ = α / (Δt²)，α 为 compliance（逆刚度）
```

---

## 四、Island 系统（物理孤岛）

```
Island = 一组相互影响的物理对象集合

Island 划分原理：
  两个对象有接触约束/关节约束 → 归入同一 Island
  Island 内的对象必须一起求解（相互影响）
  不同 Island 之间完全独立（可并行求解）

Island 图示：

  Island A:           Island B:           Island C:
  ┌──────────┐        ┌──────┐            ┌────┐
  │Box─Box   │        │Chain │            │Ball│
  │  └─Box   │        │linked│            └────┘
  └──────────┘        └──────┘
  （3个相互              （5个链接             （单独静止
    接触的箱子）            的物体）              小球）

  A 和 B 在独立 Task 中并行求解，不互相等待

Island 合并与分裂：
  合并：两个 Island 的对象开始接触 → 合并为一个大 Island
  分裂：Island 内对象失去所有接触 → 分裂为多个 Island
  睡眠：Island 内所有对象速度低于阈值 → 整个 Island 进入 Sleep
  唤醒：Island 内任一对象被施力/碰撞 → 整个 Island 唤醒

性能影响：
  大型 Island（100+ 对象接触） → 无法并行，性能瓶颈
  → 设计关卡时避免创建超大物理堆叠
  → 使用 Sleep 减少活跃 Island 数量
```

---

## 五、Task Graph 多线程模型

```
Chaos 使用 UE Task Graph 系统并行物理计算：

每个物理帧的 Task 依赖图：

[Game Thread: Push Commands]
          │
          ▼
[Physics Thread: AdvanceOneTimeStep]
  ├── [BroadPhase: AABB Tree 更新]
  │         │ 输出：潜在碰撞对列表
  ├── [NarrowPhase: 精确碰撞检测]
  │         │ 输出：Contact Constraints
  ├── [Island 划分]
  │         │
  ├── [并行 Island 求解]（每个 Island 一个 Task）
  │   ├── Island A Solver Task
  │   ├── Island B Solver Task
  │   └── Island C Solver Task
  ├── [Constraint 后处理]
  └── [Integration（位置/速度更新）]
          │
          ▼
[Physics Thread: Write Results to Proxy]
          │
          ▼
[Game Thread: Pull Results（PhysicsEndCallback）]

线程数量：
  物理线程池大小 = min(CPU核心数 - 2, 4)（默认）
  可通过 p.Chaos.ThreadingMode 控制：
    0 = TaskGraph（默认，推荐）
    1 = DedicatedThread（专用物理线程）
    2 = SingleThread（调试用）
```

---

## 六、碰撞检测流水线

```
BroadPhase（宽相碰撞）：
  目标：快速筛选可能碰撞的对象对，剔除不可能碰撞的
  算法：AABB Tree（轴对齐包围盒树）
  每帧更新：移动对象的 AABB → 树重新平衡
  
  性能：O(n log n)，每对象每帧约 0.001ms

NarrowPhase（窄相碰撞）：
  目标：对 BroadPhase 输出的候选对精确检测
  算法：GJK（Gilbert-Johnson-Keerthi）+ EPA（EPA 算法）
  支持形状：球/胶囊/盒/凸包/三角 Mesh/高度场
  
  GJK 原理：
    判断两个凸体的 Minkowski 差是否包含原点
    包含原点 → 两体相交
    不包含 → 计算最近距离
  
  EPA（EPA）：
    GJK 检测到相交后
    计算最小穿透向量（MTV）
    → 用于生成碰撞响应

  性能：凸包 vs 凸包 约 0.01~0.05ms/对

Contact Manifold（接触流形）：
  碰撞点的集合（通常 1~4 个点）
  每个接触点包含：
    位置（World Space）
    法线（从 B 指向 A）
    穿透深度
    摩擦系数
  用于约束求解的输入
```

---

## 七、Chaos 调试接口

```
关键调试命令：

// 物理线程模式
p.Chaos.ThreadingMode 0/1/2

// 碰撞调试
p.Chaos.DebugDraw.ShowCollision 1
p.Chaos.DebugDraw.ShowIslands 1
p.Chaos.DebugDraw.ShowContacts 1
p.Chaos.DebugDraw.ShowConstraints 1

// 性能统计
stat ChaosPhysics          ← 物理帧时间
stat ChaosCollision        ← 碰撞检测时间
stat ChaosConstraints      ← 约束求解时间

// Chaos Visual Debugger（CVD）
在 Project Settings → Physics → Chaos Visual Debugger 启用
运行时录制物理状态 → 在 CVD 工具中逐帧回放分析
```

---

## 八、延伸阅读

- 📄 [1.3 刚体基础概念](./03-rigid-body-basics.md)
- 🔗 [Chaos 源码](https://github.com/EpicGames/UnrealEngine/tree/ue5-main/Engine/Source/Runtime/Chaos)
- 🔗 [XPBD 论文：Müller et al. 2020](https://matthias-research.github.io/pages/publications/XPBD.pdf)
- 🔗 [PBD 原论文：Müller et al. 2007](https://matthias-research.github.io/pages/publications/posBasedDyn.pdf)
