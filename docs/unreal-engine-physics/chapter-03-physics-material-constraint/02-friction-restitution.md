# 3.2 摩擦力与弹性（Friction & Restitution）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 40 分钟

---

## 一、摩擦力物理模型

```
库仑摩擦模型（Coulomb Friction）：
  静摩擦力（Static Friction）：
    F_static ≤ μ_s * N
    物体静止时，阻止开始滑动的力
    μ_s = 静摩擦系数
    N = 法向力（垂直于接触面的压力）
    
  动摩擦力（Dynamic/Kinetic Friction）：
    F_kinetic = μ_k * N
    物体已经滑动后，减缓滑动的力
    μ_k ≤ μ_s（动摩擦通常 ≤ 静摩擦）
    
  UE 中的简化：
    Friction = 同一参数同时用于 Static 和 Dynamic
    或分别设置 Static Friction / Friction（Dynamic）

PBD 中的摩擦处理：
  Chaos 使用基于位置的摩擦约束
  不直接计算摩擦力，而是在约束投影时限制切向位移
  摩擦锥（Friction Cone）：切向冲量不超过 μ * 法向冲量
  
  |J_tangential| ≤ μ * |J_normal|
  超出 → 滑动摩擦（Dynamic Friction）
  未超出 → 静摩擦（无滑动）
```

---

## 二、摩擦力对游戏的实际影响

```
摩擦力过低（冰面 μ≈0.02）：
  角色难以急停（滑行很远）
  车辆无法转弯（打滑）
  物体推不住（一推就滑走）
  
摩擦力过高（橡胶 μ≈0.9）：
  物体触地后立即停止（没有"滑感"）
  车辆急停距离极短
  角色移动很"粘"
  
调优建议（角色移动）：
  地面 Friction = 0.6~0.8  （正常跑动控制良好）
  冰面 Friction = 0.02~0.05（滑冰效果）
  角色胶囊 Friction = 0.0（让 CharacterMovementComponent 自己处理！
    胶囊不应该有摩擦力，否则和地面产生意外旋转力矩）

角色胶囊的特殊处理：
  UCharacterMovementComponent 用自己的速度模型控制移动
  胶囊物理材质 Friction 应设为 0（避免冲突）
  地面摩擦感通过 CMC 的 BrakingFriction/GroundFriction 控制
  
  // 正确：地面有摩擦，胶囊没有
  CapsuleComp->PhysMaterialOverride = NoFrictionMaterial; // Friction=0
  // CMC 控制减速
  CMC->GroundFriction = 8.0f;
  CMC->BrakingDecelerationWalking = 2048.f;
```

---

## 三、弹性（Restitution）物理模型

```
弹性恢复系数 e（Coefficient of Restitution）：
  e = 碰撞后相对速度 / 碰撞前相对速度（绝对值比）
  
  e = 0.0：完全非弹性（粘性碰撞）
    碰撞后不分离，像橡皮泥粘在一起
    示例：子弹射入沙袋，球落地不弹起
  
  e = 1.0：完全弹性（理想弹性，能量守恒）
    碰撞后相对速度完全反转
    现实中不存在（总有能量损失）
    示例：超弹力球
  
  e ∈ (0,1)：非完全弹性（现实情况）
    示例：网球 e≈0.7，钢球 e≈0.6，木球 e≈0.5

碰撞后速度计算：
  v1' = (m1*v1 + m2*v2 + m2*e*(v2-v1)) / (m1+m2)
  v2' = (m1*v1 + m2*v2 + m1*e*(v1-v2)) / (m1+m2)
  
  特例：m2=∞（地面/固定物体）：
    v1' = -e * v1（仅速度方向反转，大小乘以 e）
    
    例：球以 v=500cm/s 落地，e=0.7：
      第1跳：v' = -0.7 * 500 = -350cm/s  （弹起高度 = 0.49 * 初始高度）
      第2跳：v' = -0.7 * 350 = -245cm/s  （弹起高度 = 0.49² = 0.24 * 初始）
      最终：弹跳高度以 e² 为公比的等比数列衰减
```

---

## 四、Bounce Threshold 与弹跳截止

```
Bounce Threshold Velocity（弹跳阈值速度）：
  Project Settings → Physics → Bounce Threshold Velocity
  默认：200 cm/s
  
  含义：当碰撞相对速度 < 此阈值时，不计算弹跳（直接静止）
  
  作用：
    防止物体在静止时无限微小弹跳（抖动）
    模拟"能量消耗到弹不起来"的物理直觉
  
  调优场景：
    弹力球游戏：降低到 50 cm/s（让球弹跳更多次）
    重型物体着陆：提高到 500 cm/s（着陆后立即静止，不抖动）
  
  与 Sleep Threshold 的关系：
    BounceThreshold 控制弹跳停止
    SleepThreshold 控制物理模拟完全停止
    两者应匹配：BounceThreshold >> SleepThreshold

实际案例：
  问题：金属球落到钢板上不断"抖动"
  原因：Bounce Threshold 太小，每次微弹都重新计算弹跳
  修复：增大 Bounce Threshold 到 100~300，
        或降低 Restitution（e从0.5改为0.2）
```

---

## 五、Combine Mode 的实用策略

```
不同 Combine Mode 的游戏应用：

冰面场景：
  冰面 PhysMat：Friction=0.02, CombineMode=Min
  任何其他材质与冰面接触 → min(0.02, x) = 0.02
  → 无论角色靴子摩擦多大，踩冰面都会滑

橡胶地面：
  橡胶 PhysMat：Friction=0.9, CombineMode=Max
  任何材质踩橡胶地面 → max(0.9, x) = 0.9
  → 无论鞋底材质，踩橡胶都有高摩擦

混合地形（沙地+石头拼接）：
  两种地面 CombineMode=Average（默认）
  边界处过渡自然

完美弹力地板（蹦床）：
  地板 PhysMat：Restitution=1.0, CombineMode=Max
  任何球都以完美弹性弹起
  → 弹力来自地板，不管球本身的 Restitution

推荐默认设置（大多数游戏）：
  Friction Combine Mode    = Average
  Restitution Combine Mode = Average
  仅特殊材质（冰/橡胶/蹦床）使用 Min/Max
```

---

## 六、延伸阅读

- 📄 [3.3 表面类型与音效](./03-surface-type.md)
- 📄 [3.4 物理约束总览](./04-constraint-overview.md)
- 🔗 [Physical Materials Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/physical-materials-reference-in-unreal-engine)
