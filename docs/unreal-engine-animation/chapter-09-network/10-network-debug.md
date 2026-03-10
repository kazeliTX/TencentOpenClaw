# 9.10 网络动画调试指南

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 20 分钟

---

## 一、网络动画常见问题

| 问题现象 | 可能原因 | 排查方向 |
|---------|---------|---------|
| 其他玩家动画不同步 | Replicated 变量未声明 | 检查 GetLifetimeReplicatedProps |
| Montage 在其他客户端不播放 | 未使用 Multicast RPC | 确认 RPC 是 NetMulticast |
| SimProxy 角色动画卡顿 | 位置平滑未开启 | p.NetEnableMoveSmoothingParam 1 |
| 攻击动画有延迟感 | 未做客户端预测 | 本地立即播放 + ServerRPC 确认 |
| Root Motion 位置不对 | SimProxy Root Motion 问题 | IgnoreRootMotion for SimProxy |
| AnimNotify 触发两次 | 服务端+客户端都执行了游戏逻辑 | 加 HasAuthority() 保护 |

---

## 二、网络调试工具

```bash
# 模拟网络延迟（本地测试用）
net.PktLag 100          ← 模拟 100ms 延迟（单向，RTT=200ms）
net.PktLoss 5           ← 模拟 5% 丢包率
net.PktLagVariance 20   ← 延迟抖动 ±20ms

# 查看网络状态
stat net                ← 网络统计（带宽/丢包率/ping）
net.showdebug           ← 显示网络调试信息

# 查看角色复制
p.VisualizeMovement 1   ← 可视化 CMC 移动（显示服务端/客户端位置差异）

# 在编辑器中运行多客户端
PIE Settings → Number of Players: 2
→ 可以同时看到 Server + Client 视角
→ 快速验证网络同步
```

---

## 三、PIE 多客户端调试流程

```
1. 设置 PIE 为 2 个玩家（Server + Client）
2. 在两个视口中观察角色动画差异
3. 使用控制台命令注入延迟：net.PktLag 100
4. 检查以下关键点：
   □ 本地移动动画流畅（AutProxy）
   □ 对方角色动画同步（SimProxy 插值正常）
   □ 攻击 Montage 在两端都播放
   □ AnimNotify 游戏逻辑只触发一次
   □ Root Motion 位置不漂移

常见测试场景：
  快速转向 → 检查速度/方向同步
  跳跃 → 检查 IsInAir 同步
  连续攻击 → 检查 Montage 队列
  高延迟（PktLag=300）→ 检查校正抖动
```

---

## 四、网络动画优化 Checklist

```
□ 移动状态同步
  □ 只 Replicate 动画需要的变量（不复制所有状态）
  □ 使用压缩类型（FVector_NetQuantize, uint8）
  □ SimulatedOnly 条件减少不必要的同步

□ Montage 同步
  □ 所有攻击/技能 Montage 通过 RPC 同步
  □ 本地预测 + 服务端确认
  □ 极少数高频 Montage 使用 Unreliable RPC

□ SimProxy 优化
  □ SimProxy 禁用 Root Motion
  □ 位置平滑开启
  □ SimProxy 使用简化 AnimGraph（更低 LOD）

□ AnimNotify 权限
  □ 游戏逻辑都有 HasAuthority() 保护
  □ 不在 AnimNotify 中发送 RPC
```

---

## 五、延伸阅读

- 🔗 [网络调试文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/network-profiler-in-unreal-engine)
- 🔗 [PIE 多客户端测试](https://dev.epicgames.com/documentation/en-us/unreal-engine/play-in-editor-settings-in-unreal-engine)
