# 3.1 AnimBP 架构与线程模型

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟
> 理解线程模型是写出正确且高性能 AnimBP 的前提

---

## 一、AnimBP 整体架构

```
Animation Blueprint 由两部分组成：

┌─────────────────────────────────────────────────────────┐
│                  Animation Blueprint                      │
│                                                           │
│  ┌────────────────────┐   ┌──────────────────────────┐  │
│  │    Event Graph      │   │       Anim Graph          │  │
│  │                     │   │                           │  │
│  │  • 读取 Gameplay 状态│   │  • 纯骨骼变换计算         │  │
│  │  • 更新 AnimBP 变量  │   │  • State Machine         │  │
│  │  • 执行逻辑判断     │   │  • Blend Nodes            │  │
│  │  • 触发事件         │   │  • IK Nodes               │  │
│  │                     │   │  • Skeletal Controls      │  │
│  │  ⚠️ 非线程安全       │   │  ✅ 线程安全              │  │
│  │  Game Thread 执行    │   │  Worker Thread 执行       │  │
│  └────────────────────┘   └──────────────────────────┘  │
└─────────────────────────────────────────────────────────┘

UAnimInstance（C++类）
  ├── NativeUpdateAnimation()     → 替代 Event Graph（Game Thread）
  ├── NativeThreadSafeUpdateAnimation() → 线程安全更新（Worker Thread）
  └── AnimGraph 节点引用 AnimInstance 的 BlueprintReadOnly 属性
```

---

## 二、双线程执行模型

### 2.1 时序图

```
每帧动画更新时序（简化版）：

Game Thread:
  ├─ [Character::Tick] 移动、输入、Gameplay 逻辑
  ├─ [SkeletalMeshComponent::TickComponent]
  │   └─ 触发动画更新任务
  └─ [UAnimInstance::NativeUpdateAnimation]
       ├─ 读取 Character 状态（速度、姿势、状态机标志）
       └─ 写入 AnimBP 变量（Speed, bIsJumping, AimPitch...）
           │
           ▼（将 AnimGraph 评估推送到工作线程）

Worker Thread（并行执行）:
  └─ [AnimGraph 评估]
       ├─ 读取 AnimBP 变量（只读！）
       ├─ 评估 State Machine（状态转换、混合权重）
       ├─ 采样动画序列
       ├─ 执行 IK / Skeletal Control
       └─ 输出 Component Space Pose
           │
           ▼（完成后回到 Game Thread）

Game Thread（合并）:
  └─ FillComponentSpaceTransforms → 写入骨骼矩阵
  └─ SendRenderDynamicData → GPU 顶点蒙皮
```

### 2.2 线程模型的关键规则

```
✅ 可以在 AnimGraph / Worker Thread 中做的：
  • 读取 AnimInstance 的 BlueprintReadOnly / BlueprintReadWrite 变量
  • 数学运算（FVector, FRotator, FTransform...）
  • 访问 Property Access 系统暴露的数据
  • 调用 ThreadSafe 标记的 BlueprintPure 函数

❌ 不能在 AnimGraph / Worker Thread 中做的：
  • Cast 到 Character/Controller 等 UObject（非线程安全）
  • 访问 Actor 组件（GetMesh()、GetMovementComponent()等）
  • 发送 GameplayEvent、修改 Actor 状态
  • 调用未标记 ThreadSafe 的函数
```

---

## 三、Property Access 系统（UE5 推荐方式）

UE5 引入 Property Access 系统，允许 AnimGraph 在工作线程中**安全读取** Gameplay 数据：

```
Property Access 工作原理：
  1. 在 AnimBP 的 Property Access 面板中配置访问路径
     例：TryGetPawnOwner → GetVelocity → Size
  2. UE 在 Game Thread 提前拷贝数据到缓存
  3. Worker Thread 读取缓存（线程安全）

配置步骤：
  AnimBP 编辑器 → Class Settings → Property Access
  + 添加新访问路径：
    Binding: TryGetPawnOwner.GetVelocity.Size  
    Cache Mode: Copy on Worker Thread（推荐）

在 AnimGraph 中使用：
  Property Access 节点 → 选择配置的路径 → 直接连接到混合参数
```

---

## 四、UAnimInstance 的执行顺序

```cpp
// 完整执行顺序（每帧）：

// 1. 初始化（仅一次）
virtual void NativeInitializeAnimation() override;

// 2. Game Thread：读取 Gameplay 状态，写入变量
virtual void NativeUpdateAnimation(float DeltaSeconds) override;

// 3. Worker Thread：线程安全计算（UE 5.0+）
virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

// 4. AnimGraph 评估（Worker Thread）
// → 由引擎内部驱动，不需要手动调用

// 5. Game Thread：后处理（动画姿势已计算完毕）
virtual void NativePostUpdateAnimation() override;

// 6. 骨骼变换写入完成后
virtual void NativePostEvaluateAnimation() override;
```

---

## 五、架构选型建议

```
纯蓝图 AnimBP：
  优点：快速原型，可视化直观
  缺点：性能较差，复杂逻辑难维护
  适用：小项目、快速验证

C++ AnimInstance + 蓝图 AnimGraph：（推荐）
  优点：逻辑在 C++ 中（快速、线程安全），AnimGraph 可视化
  缺点：需要 C++ 基础
  适用：中大型项目

全 C++ AnimInstance：
  优点：最高性能
  缺点：失去可视化调试能力
  适用：极端性能场景（如上千个 NPC）

Linked AnimGraph 模块化：（UE5 推荐）
  优点：逻辑拆分，复用性高，团队协作友好
  缺点：调试链路增加
  适用：大型项目，多人协作
```

---

## 六、延伸阅读

- 📄 [3.2 Event Graph 变量更新](./02-event-graph.md)
- 📄 [3.7 C++ AnimInstance 完整实现](./07-anim-instance-cpp.md)
- 🔗 [Animation Blueprint Overview](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprints-in-unreal-engine)
- 🔗 [多线程动画更新（知乎）](https://zhuanlan.zhihu.com/p/381967985)
