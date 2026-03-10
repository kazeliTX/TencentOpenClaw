# 2.4 Collision Profile 设计与管理

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、内置 Collision Profiles

```
UE 内置常用 Profile（Project Settings → Collision → Preset）：

NoCollision：
  Collision Enabled：NoCollision
  所有通道：Ignore

BlockAll：
  Collision Enabled：QueryAndPhysics
  Object Type：WorldDynamic
  所有通道：Block

OverlapAll：
  Collision Enabled：QueryOnly
  Object Type：WorldDynamic
  所有通道：Overlap

BlockAllDynamic：
  阻挡所有动态对象，忽略 WorldStatic

Pawn：
  Object Type：Pawn
  WorldStatic：Block
  WorldDynamic：Block
  Pawn：Block（同类阻挡）
  PhysicsBody：Overlap
  Camera：Ignore
  Visibility：Block

PhysicsActor：
  Object Type：PhysicsBody
  WorldStatic：Block
  WorldDynamic：Block
  Pawn：Block
  PhysicsBody：Block

Trigger：
  Object Type：WorldDynamic
  Collision Enabled：QueryOnly
  Pawn：Overlap
  其他：Ignore

Ragdoll：
  Object Type：PhysicsBody
  WorldStatic：Block
  WorldDynamic：Block
  Pawn：Ignore（不阻挡自身角色）
  PhysicsBody：Block
```

---

## 二、自定义 Profile 完整配置流程

```
在 Project Settings → Collision → Preset 中添加：

步骤：
  1. New Preset → 设置 Profile Name（如 "Bullet"）
  2. Collision Enabled：QueryAndPhysics（子弹需要物理）
  3. Object Type：GameTraceChannel1（自定义 Projectile 通道）
  4. 通道响应逐一配置：
     WorldStatic → Block（打墙会停下）
     WorldDynamic → Block（打动态物体会停下）
     Pawn → Block（打角色会停下，触发 Hit）
     PhysicsBody → Block
     Visibility → Ignore（射线检测不用触发）
     Camera → Ignore
     Bullet（自身）→ Ignore（子弹不打子弹）

  5. 保存到 DefaultEngine.ini

在 C++ 中引用：
  Comp->SetCollisionProfileName(FName("Bullet"));

在蓝图中引用：
  Mesh → Details → Collision → Collision Presets → "Bullet"

常见自定义 Profile 设计：
  ────────────────────────────────────────────────────────
  Profile          ObjectType      WS  WD  Pawn  PB  Camera
  ────────────────────────────────────────────────────────
  Bullet           Projectile      B   B   B     B   I
  Melee            WorldDynamic    I   I   O     I   I
  TriggerZone      WorldDynamic    I   I   O     O   I
  Ragdoll          PhysicsBody     B   B   I     B   I
  PickupItem       WorldDynamic    I   I   O     I   I
  WaterVolume      WorldDynamic    I   I   O     I   I
  ────────────────────────────────────────────────────────
  (B=Block, O=Overlap, I=Ignore, WS=WorldStatic, WD=WorldDynamic, PB=PhysicsBody)
```

---

## 三、Profile 与响应组合最佳实践

```
设计原则：

原则 1：最小化 Block
  只在真正需要物理阻挡时使用 Block
  过多 Block 关系 → 约束求解量增加 → 性能下降

原则 2：拾取/触发使用 QueryOnly + Overlap
  拾取物不需要物理模拟
  只需检测角色进入范围 → QueryOnly 性能更好

原则 3：子弹/抛射物使用 CCD
  高速 + 小尺寸 → 必须开 CCD
  或使用射线 Trace 代替物理子弹（更推荐）

原则 4：相同类型对象通常 Ignore 彼此
  子弹 vs 子弹 → Ignore
  NPC vs NPC → 可以 Ignore（减少 AI 拥挤计算）

原则 5：角色与触发器分离处理
  Pawn Profile 对 Trigger 通道设置 Overlap
  Trigger Profile 对 Pawn 通道设置 Overlap
  这样进入/离开触发区域时自动触发 Overlap 事件

运行时 Profile 切换示例（角色死亡变布娃娃）：
  // 生存状态
  Mesh->SetCollisionProfileName("Pawn");
  
  // 死亡 → 切换为 Ragdoll Profile
  Mesh->SetCollisionProfileName("Ragdoll");
  Mesh->SetSimulatePhysics(true);
```

---

## 四、多 Component 的碰撞管理

```
一个 Actor 通常有多个 Component，各自可以有不同 Profile：

示例：武装角色
  CapsuleComponent：
    Profile = "Pawn"（主体碰撞，阻挡移动）
    处理移动阻挡、地面检测
  
  SkeletalMeshComponent（身体）：
    Profile = "CharacterMesh"
    对 Pawn = Ignore（不阻挡自身胶囊）
    对 WorldStatic = Block
    对 Visibility = Block（被子弹打到）
    对 Camera = Ignore
  
  WeaponMesh：
    Profile = "Weapon"
    对 Pawn = Overlap（记录近战命中但不阻挡）
    对 WorldStatic = Block
  
  HitboxComponent（头部专属伤害判定）：
    Profile = "DamageHitbox"（自定义）
    CollisionEnabled = QueryOnly
    对 WeaponTrace = Overlap
    其他 = Ignore
    → 只响应武器 Trace，不参与物理阻挡

优先级：
  如果 Component 设置了具体 Profile
  → 覆盖 Actor 级别的设置
  Actor.SetActorEnableCollision(false) → 关闭所有 Component 碰撞
```

---

## 五、延伸阅读

- 📄 [2.5 Hit Event 详解](./05-hit-events.md)
- 📄 [代码示例：碰撞设置](./code/01_collision_setup.cpp)
- 🔗 [Collision Presets](https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-response-reference-in-unreal-engine)
