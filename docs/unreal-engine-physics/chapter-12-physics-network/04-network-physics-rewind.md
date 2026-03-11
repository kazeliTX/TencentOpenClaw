# 12.4 Physics Rewind（物理回卷调试）

> **难度**：⭐⭐⭐⭐⭐

```
Physics Rewind（UE5.1+ 调试工具）：
  录制并回放物理模拟历史
  用于调试网络不同步问题
  
启用：
  p.PhysicsReplication.Resimulation.EnableRewind 1
  
录制：
  p.ChaosVisualDebugger.CaptureMode 1   ← 开始录制
  p.ChaosVisualDebugger.CaptureMode 0   ← 停止录制
  
回放：
  Chaos Visual Debugger 工具面板（Editor 工具栏）
  → Load Recording → 时间轴播放/逐帧回放
  → 对比服务端 vs 客户端状态差异

诊断工作流：
  1. 复现不同步问题（两个客户端同时与物体交互）
  2. 录制服务端物理历史（p.ChaosVisualDebugger）
  3. 录制客户端物理历史
  4. 在 Chaos Visual Debugger 中加载两份记录
  5. 对比时间轴，找到第一个出现差异的帧
  6. 检查该帧的输入数据/初始状态/碰撞事件
  7. 根据差异来源调整：延迟补偿/回滚阈值/复制频率
```
