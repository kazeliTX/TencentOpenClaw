# 8.9 性能优化

> **难度**：⭐⭐⭐⭐☆

## 核心内容

```
性能建议：
  PhysicalAnimation 每帧对每个 Body 施加弹簧力
  开销 ≈ Ragdoll × 0.5（弹簧计算比完整约束便宜）

优化策略：
  1. 只对必要骨骼应用（spine_01 以上即可，不需要手指）
  2. 非可见/远距离角色：SetStrengthMultiplier(0) 停止追随
  3. 静止时降低强度（角色站立不动时几乎不需要物理动画）
  4. 命中后才激活（默认 Kinematic，命中时开启物理 + PhysAnim）

距离优化示例：
  void Tick(float Dt)
  {
      float D = DistanceToPlayer();
      if (D > 3000.f)
      {
          GetMesh()->SetAllBodiesSimulatePhysics(false);
          return;
      }
      float Strength = D > 1500.f ?
          FMath::Lerp(1.f, 0.f, (D - 1500.f) / 1500.f) : 1.f;
      PhysAnim->SetStrengthMultiplier(Strength);
  }

Profile 激活数量建议：
  同屏 PhysicalAnimation 角色 ≤ 16（主机）
  ≤ 32（高端 PC）
  超出时对最远角色关闭物理动画
```
