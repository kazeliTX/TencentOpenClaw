# 12.8 车辆网络同步

> **难度**：⭐⭐⭐⭐⭐

```
ChaosVehicle 网络同步原生支持（UE5）：

核心机制：
  ChaosWheeledVehicleMovementComponent 继承自
  UNetworkPhysicsComponent
  → 内置输入预测/回滚框架
  
输入同步：
  客户端输入（油门/转向/刹车）通过
  Server RPC 发送到服务端
  服务端计算权威物理结果
  
状态同步：
  FReplicatedVehicleState 结构体：
    Position / Rotation / LinearVelocity / AngularVelocity
    CurrentGear / ThrottleInput / BrakeInput / SteeringInput
  通过 RepNotify 广播给所有客户端
  
非本地玩家车辆显示：
  其他玩家的车辆：收到状态后平滑插值（RemoteRole = SimulatedProxy）
  插值方式：对位置/速度做 Hermite 样条插值
  → 即使网络不稳定也不会出现"卡步"
  
推荐配置：
  NetUpdateFrequency = 60.f（高速运动）
  MinNetUpdateFrequency = 10.f（静止或慢速）
  bReplicatePhysicsToAutonomousProxy = true（同步回本地预测）
```
