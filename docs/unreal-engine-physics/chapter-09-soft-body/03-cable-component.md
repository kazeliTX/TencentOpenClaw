# 9.3 UCableComponent 详解

> **难度**：⭐⭐⭐☆☆

## 一、UCableComponent 参数

```cpp
// 配置一根垂落装饰电缆
UCableComponent* Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
Cable->CableLength       = 200.f;   // 绳索自然长度（cm）
Cable->NumSegments       = 12;      // 分段数（越多越平滑，越慢）
Cable->SolverIterations  = 8;       // 求解迭代（越多越准确，越慢）
Cable->CableWidth        = 1.5f;    // 渲染宽度
Cable->SubstepTime       = 0.02f;   // 子步长（小=稳定，大=性能好）

// 两端附加
Cable->SetAttachEndTo(OtherActor, NAME_None, FName("AttachSocket"));
Cable->AttachEndToSocketName = FName("SocketB");

// 重力方向上的自然下垂形状（无需物理模拟也能看起来自然）
Cable->bEnableStiffness  = true;    // 启用硬度（减少穿模）
Cable->StiffnessCoefficient = 50.f; // 硬度系数
```

## 二、性能 vs 效果权衡

```
NumSegments 推荐值：
  纯装饰/远景：4~6 段（极快）
  中景可见：8~12 段（平衡）
  近景特写：16~24 段（精细）

SolverIterations：
  2~4：快速近似（大多数情况够用）
  8：标准质量
  16：高精度（吊桥/绳梯等重要道具）

电线杆之间的电线（纯视觉，数量多）：
  NumSegments = 4, SolverIterations = 2
  → 一根开销极小（< 0.01ms）
  → 200 根同屏也只有约 2ms

注意：UCableComponent 不支持碰撞响应
  → 不能用作可攀爬绳索
  → 不能准确阻挡玩家
```

## 三、延伸阅读

- 📄 [9.4 物理绳索链条](./04-physics-rope.md)
