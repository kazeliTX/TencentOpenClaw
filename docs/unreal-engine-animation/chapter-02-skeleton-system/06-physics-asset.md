# 2.6 物理资产（Physics Asset）完全指南

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 35 分钟

---

## 一、Physics Asset 的作用

Physics Asset（PhAT，物理资产）定义了骨骼角色的：
1. **物理形体（Physics Bodies）**：每根骨骼的碰撞体（球体/胶囊体/盒体）
2. **物理约束（Physics Constraints）**：骨骼间的关节约束（角度限制、阻尼等）
3. **布娃娃配置**：死亡/击倒时的物理行为

```
Physics Asset 在角色中的用途：

用途                          说明
────────────────────────────────────────────────────
布娃娃（Ragdoll）            死亡/昏迷时启用完全物理模拟
物理动画混合                  动画与物理的混合（受击晃动等）
布料模拟                      布料与骨骼碰撞
IK 脚部碰撞                   脚部 IK 的地面检测基准体
子弹/爆炸冲击力应用            物理冲量作用到角色身体部位
```

---

## 二、Physics Asset 编辑器（PhAT）

```
打开方式：双击 Physics Asset 资产
或：在 Skeletal Mesh 编辑器 → Physics 标签

编辑器界面：
┌─────────────────────────────────────────────────────┐
│ 工具栏：[Body Mode] [Constraint Mode] [Simulate]    │
├───────────────┬─────────────────────┬───────────────┤
│ 骨骼/Body 树  │      3D 视口         │  Details 面板 │
│               │   [角色形体预览]      │               │
│ ▶ root        │                     │  选中 Body 的 │
│   ▶ pelvis    │  碰撞体为半透明形状   │  属性设置      │
│     ▶ spine   │  约束为彩色线条      │               │
│     ...       │                     │               │
└───────────────┴─────────────────────┴───────────────┘
```

---

## 三、Physics Body（物理形体）配置

### 3.1 形体类型选择

| 形体类型 | 适用骨骼 | 性能开销 | 精度 |
|---------|---------|---------|------|
| Capsule（胶囊体）| 四肢、躯干 | 最低 | 中等 |
| Sphere（球体）| 头部、关节 | 最低 | 较低 |
| Box（盒体）| 躯干、盔甲 | 低 | 中等 |
| TaperedCapsule | 锥形肢体 | 低 | 较高 |
| Convex Hull | 特殊形状 | 高 | 高 |

### 3.2 常用设置

```
Body Details（选中 Physics Body 后在 Details 面板看到）：

Physics Type:
  • Simulated：完全物理模拟（布娃娃模式）
  • Kinematic：跟随动画，不受物理影响（默认动画模式）
  • Default：跟随父 Body 的设置

Collision Response:
  • Enabled：参与碰撞检测
  • Disabled：忽略碰撞（减少性能开销）
  
Linear Damping:  0.0（线性阻尼，影响平移运动的衰减）
Angular Damping: 0.05（角度阻尼，影响旋转的衰减）
Mass:            1.0（质量，影响冲量响应）
```

---

## 四、Physics Constraint（物理约束）配置

约束定义了相邻骨骼之间的运动限制：

```
约束类型示意：

髋关节（Ball and Socket）：
  角度自由度：Swing1 ±45°, Swing2 ±45°, Twist ±90°
  模拟：大腿可以在一定范围内旋转，但不能无限制

膝关节（Hinge）：
  角度自由度：Swing1 = 0°（锁死）, Swing2 = 0°（锁死）, Twist: -5°~120°
  模拟：膝盖只能前后弯曲，不能左右扭转

脊椎：
  角度自由度：Swing1 ±20°, Swing2 ±10°, Twist ±15°
  模拟：脊椎有一定弯曲范围，但比髋关节受限更多
```

### Constraint Detail 参数

```
Linear Limits:
  • XMotion/YMotion/ZMotion: Free / Limited / Locked
  • Linear Limit Size: 限制距离（cm）

Angular Limits:
  Swing 1 Motion: Free / Limited / Locked
    Swing 1 Limit Angle: 45.0°（最大摆动角度）
  Swing 2 Motion: Limited
    Swing 2 Limit Angle: 45.0°
  Twist Motion: Limited
    Twist Limit Angle: 90.0°（最大扭转角度）

Drive（弹簧）：
  Orientation Drives:
    • Slerp Drive: 弹簧驱动，让约束有弹性
    • Strength: 弹力强度
    • Damping: 阻尼（防止震荡）
  用途：物理动画混合时，让骨骼有弹性跟随动画
```

---

## 五、物理资产的自动生成

```
快速生成：
  Physics Asset 编辑器 → 工具栏 → Body Creation

推荐设置（角色）：
  Minimum Bone Size: 5.0cm（小于此长度的骨骼跳过）
  Body Creation Method: Capsule Per Bone（每骨骼一个胶囊体）
  Constrain Bones: ✅（自动生成约束）
  Walk Past Small Bodies: ✅（跳过极小骨骼）

自动生成后通常需要手动调整：
  • 头部改为球体
  • 调整躯干胶囊体大小
  • 调整约束角度限制
  • 移除手指等细小骨骼的 Body（布娃娃通常不需要）
```

---

## 六、C++ 配置物理资产

```cpp
// 开启/关闭全身物理（布娃娃模式）
void EnableRagdoll(USkeletalMeshComponent* Mesh)
{
    Mesh->SetSimulatePhysics(true);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    // 可选：给初始冲量
    Mesh->AddImpulse(FVector(0, 0, -500.0f), NAME_None, true);
}

void DisableRagdoll(USkeletalMeshComponent* Mesh)
{
    Mesh->SetSimulatePhysics(false);
    // 重新连接到动画
    Mesh->AttachToComponent(
        Mesh->GetAttachParent(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale
    );
}

// 给特定骨骼施加冲量（如子弹击中）
void ApplyBulletImpact(
    USkeletalMeshComponent* Mesh,
    FName HitBoneName,
    FVector ImpactDirection,
    float ImpulseStrength = 50000.0f)
{
    FVector Impulse = ImpactDirection.GetSafeNormal() * ImpulseStrength;
    Mesh->AddImpulse(Impulse, HitBoneName, true); // true = vel change
}

// 查询 Body 当前状态
bool IsBoneSimulating(USkeletalMeshComponent* Mesh, FName BoneName)
{
    if (!Mesh) return false;
    FBodyInstance* BodyInst = Mesh->GetBodyInstance(BoneName);
    return BodyInst && BodyInst->IsInstanceSimulatingPhysics();
}
```

---

## 七、性能建议

```
物理 Body 数量 vs 性能参考（每帧）：

10 个 Body（简化布娃娃）：  0.1-0.3ms
20 个 Body（标准角色）：    0.3-0.8ms  ← 推荐上限
40 个 Body（带手指）：      0.8-2.0ms

优化建议：
  1. 布娃娃不需要手指骨骼 Body（删除手指/脚趾 Body）
  2. 角色死亡后延迟 3-5 秒销毁布娃娃
  3. 屏幕外的布娃娃降低物理更新频率
  4. 使用 Physics LOD：远处角色用更简化的物理资产
```

---

## 八、延伸阅读

- 📄 [2.7 布娃娃系统实现](./07-ragdoll-system.md)
- 📄 [2.9 Chaos 布料模拟](./09-cloth-simulation.md)
- 🔗 [Physics Asset 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-asset-editor-in-unreal-engine)
- 🔗 [Physical Animation Component](https://dev.epicgames.com/documentation/en-us/unreal-engine/physical-animation-in-unreal-engine)
