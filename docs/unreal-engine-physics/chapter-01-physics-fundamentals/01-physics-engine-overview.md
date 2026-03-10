# 1.1 物理引擎总览（PhysX → Chaos）

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 20 分钟

---

## 一、UE 物理引擎历史

```
UE4（2014~2022）：PhysX（NVIDIA 开发）
  成熟稳定，行业标准
  不开源，Epic 无法深度定制
  UE5 开始逐步弃用

UE5（2022~至今）：Chaos（Epic 自研）
  完全开源，与引擎深度集成
  支持大规模破碎（Chaos Fracture）
  支持高质量布料（Chaos Cloth）
  支持软体/流体（实验性）
  专为 Nanite + Lumen 场景优化

过渡期（UE5.0~5.2）：
  Chaos 作为默认物理引擎
  PhysX 作为备选（部分平台）
  UE5.3+：PhysX 完全移除
```

---

## 二、Chaos 的核心特性

```
1. 统一物理求解器
   刚体 + 布料 + 破碎 + 流体 在同一框架下
   统一的碰撞检测与响应
   
2. 多线程物理（Physics Task Graph）
   物理计算在独立线程池中并行执行
   与渲染线程解耦（物理帧 ≠ 渲染帧）

3. 确定性（Deterministic）
   相同输入 → 相同输出（重要！网络同步依赖此特性）
   但目前仅在单线程模式下完全确定
   多线程模式存在微小浮点差异

4. 大世界支持
   与 UE5 Large World Coordinates（LWC）集成
   double 精度坐标，解决远离原点的物理抖动
```

---

## 三、物理引擎在 UE 中的位置

```
UE 架构层级：

游戏逻辑层（Gameplay）
    ↓ Actor / Component
物理接口层（Physics Interface）
    ↓ IPhysicsInterface（抽象层）
Chaos 物理引擎（Chaos::FPhysicsSolver）
    ↓
操作系统 / 硬件
    CPU 多线程（物理计算）
    GPU（Niagara Fluids 等）

关键模块：
  Engine/Source/Runtime/PhysicsCore/    ← 物理接口
  Engine/Source/Runtime/Chaos/          ← Chaos 实现
  Engine/Source/Runtime/Engine/Physics/ ← 引擎物理组件
```

---

## 四、物理引擎工作流程（每帧）

```
游戏线程（Game Thread）：
  1. 收集本帧物理输入（力、冲量、速度修改）
  2. 将输入写入 Chaos Physics Scene

物理线程（Physics Thread）：
  3. 碰撞检测（BroadPhase → NarrowPhase）
  4. 约束求解（Joints、Contacts）
  5. 积分（更新位置和速度）
  6. 输出结果到 Physics Proxy

游戏线程（下一帧开始时）：
  7. 从 Physics Proxy 读取结果
  8. 更新 Actor/Component 变换
  9. 触发碰撞/Overlap 事件回调

注意：物理结果有 1 帧延迟（物理线程异步执行）
```

---

## 五、延伸阅读

- 📄 [1.2 Chaos 引擎架构详解](./02-chaos-architecture.md)
- 🔗 [Chaos 物理官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-physics-in-unreal-engine)
