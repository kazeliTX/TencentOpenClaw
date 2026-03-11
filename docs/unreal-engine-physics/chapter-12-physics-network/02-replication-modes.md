# 12.2 复制模式（权威/预测）

> **难度**：⭐⭐⭐⭐⭐

## 一、EPhysicsReplicationMode

```cpp
// UE5 物理复制模式
enum class EPhysicsReplicationMode : uint8
{
    Default,         // 服务端权威：客户端用位置插值追赶服务端
    PredictiveInterpolation,  // 预测插值：结合预测和插值
    Resimulation,    // 重模拟：客户端回卷重跑（最精确，开销最大）
};

// 设置复制模式
UPrimitiveComponent* Comp = ...;
if (FBodyInstance* BI = Comp->GetBodyInstance())
{
    BI->PhysicsReplicationMode = EPhysicsReplicationMode::PredictiveInterpolation;
}

// 或在 Actor Details 中设置：
// Physics → Physics Replication Mode
```

## 二、各模式对比

```
Default（位置硬追）：
  客户端收到服务端位置后，直接线性插值到目标位置
  优点：简单/低开销
  缺点：高延迟时物体"跳动"，物理感失真
  适合：装饰性物体（木桶/椅子），不重要的互动物体

PredictiveInterpolation（预测插值）：
  客户端预测物体下一帧位置（基于当前速度+加速度）
  收到服务端数据后平滑修正
  适合：大多数物理交互场景（推箱子/车辆）

Resimulation（回卷重模拟）：
  客户端保存过去 N 帧的物理状态快照
  收到服务端校正时：回滚到服务端时间戳 → 重新模拟到当前帧
  优点：最接近服务端结果，没有明显跳变
  缺点：开销最大（重跑N帧物理）
  适合：竞技游戏中玩家控制的重要物体
```
