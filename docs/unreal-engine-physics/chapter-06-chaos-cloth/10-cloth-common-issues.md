# 6.10 常见问题与调试指南

> **难度**：⭐⭐⭐⭐☆

## 一、问题速查表

| 症状 | 最可能原因 | 修复方案 |
|------|-----------|---------|
| 布料完全不动 | 所有顶点 MaxDistance=0 | 检查权重图，确保运动区域有正值 |
| 布料穿透角色身体 | Backstop 未设置或半径太小 | 绘制 Backstop，增大 Collision Thickness |
| 布料传送后被拉伸 | 未调用 TeleportReset | `ForceClothNextUpdateTeleportReset()` |
| 布料持续抖动 | 阻尼太低或刚度太高 | ↑ Damping, ↓ Stiffness, ↑ SleepThreshold |
| 布料下摆剧烈反弹 | 与地面碰撞弹性太高 | 降低 PhysicalMaterial 弹性系数 |
| 布料在高速运动时爆炸/飞出 | 子步长太大导致数值不稳定 | ↑ Num Substeps 或 ↑ Num Iterations |
| 布料"果冻感" | Stretch/Bend 刚度太低 | ↑ Stretch Stiffness, ↑ Bend Stiffness |
| 布料贴在角色身上动不了 | Animation Weight 太高 | ↓ Animation Weight（接近0） |
| 裙摆穿进大腿 | Physics Asset 腿部 Body 太小 | 增大 thigh Body 半径并确保 Body 覆盖整条大腿 |
| 自碰撞没效果 | Self Collision Thickness 太小 | 增大至布料厚度的 1/2~1 |

## 二、调试命令大全

```
布料可视化（PIE 中输入）：
  p.ChaosCloth.DebugDraw.PhysMeshWireframe 1  ← 布料网格线框
  p.ChaosCloth.DebugDraw.Normals 1            ← 法线
  p.ChaosCloth.DebugDraw.CollisionParticles 1 ← 碰撞粒子
  p.ChaosCloth.DebugDraw.SelfCollision 1      ← 自碰撞约束
  p.ChaosCloth.DebugDraw.MaxDistances 1       ← MaxDistance 热力图
  p.ChaosCloth.DebugDraw.Backstops 1         ← Backstop 球体
  p.ChaosCloth.DebugDraw.Gravity 1           ← 重力向量
  p.ChaosCloth.DebugDraw.Wind 1              ← 风力向量

全局开关：
  p.ChaosCloth.Enable 0    ← 关闭所有布料（性能测试）
  p.ChaosCloth.Enable 1    ← 重新开启

性能统计：
  stat ChaosCloth          ← 布料帧时间统计
  stat Physics             ← 物理总帧时间
```

## 三、数值稳定性原则

```
Chaos Cloth 数值爆炸预防：

1. 子步数足够：
   Num Substeps >= 1，高速角色建议 2~3

2. 质量不要太小：
   UniformMass >= 0.001 kg（太小会导致数值不稳定）

3. 刚度和阻尼配对：
   高刚度 → 高阻尼（否则欠阻尼振荡）
   Stiffness = 1.0 → Damping >= 0.05

4. 避免瞬间大位移：
   角色传送/出生时调用 TeleportReset
   避免用 SetActorLocation 跳跃式移动（用插值或 Teleport）
```

## 四、延伸阅读

- 📄 [代码示例：布料运行时控制](./code/01_cloth_runtime_control.cpp)
- 🔗 [Chaos Cloth 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-cloth-in-unreal-engine)
