# 12.10 网络物理调试

> **难度**：⭐⭐⭐⭐⭐

## 一、常用调试命令

```
网络模拟（在 PIE 中测试延迟/丢包）：
  Editor → 编辑 → 项目设置 → 网络 → Network Emulation：
    设置 Latency（延迟 ms）、Packet Loss（丢包率）
    
  或通过命令行：
    net.PktLag 100          ← 模拟 100ms 延迟
    net.PktLoss 5           ← 模拟 5% 丢包
    net.PktLagVariance 20   ← 延迟波动 ±20ms

物理复制调试：
  p.PhysicsReplication.EnableReplicationLog 1   ← 开启复制日志
  p.PhysicsReplication.DrawDebug 1              ← 绘制服务端/客户端位置差异
  p.net.PhysicsResimulation.DebugDraw 1         ← 回滚可视化

查看物理状态不同步：
  stat Net              ← 网络带宽/数据包统计
  stat Physics          ← 物理帧时间
  p.Chaos.DebugDrawSolverStatistics 1  ← Chaos 求解器统计
```

## 二、常见网络物理 Bug 及解法

```
Bug                          原因                        解法
─────────────────────────────────────────────────────────────────────
物体在客户端跳动              复制频率低/PredictiveInterp 差  提高 NetUpdateFrequency
物体位置服务端/客户端不一致    浮点精度差异累积               加大回滚阈值 / Resimulation 模式
高延迟时物体"传送"           DefaultMode 追赶过快            改用 PredictiveInterpolation
布娃娃各客户端位置不同        正常（每端本地模拟）           接受差异，同步"死亡事件"而非状态
车辆在其他客户端抖动          速度快但更新率低               NetUpdateFreq >= 30，启用预测插值
推箱子时客户端延迟感          缺少客户端预测                 本地先 AddForce，Server RPC 权威同步
```
