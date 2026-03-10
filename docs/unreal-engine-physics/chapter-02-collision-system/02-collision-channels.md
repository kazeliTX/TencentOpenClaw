# 2.2 碰撞通道（Collision Channel）系统

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 40 分钟

---

## 一、碰撞通道概念

```
碰撞通道（Collision Channel）：
  UE 用通道来分类物理对象和查询请求
  每个通道对应一个"身份标识"
  两个对象之间是否发生碰撞，取决于各自的通道响应设置

两种通道类型：
  1. Object Channel（对象通道）：
     描述"这个对象是什么类型"
     例：WorldStatic / WorldDynamic / Pawn / PhysicsBody
  
  2. Trace Channel（追踪通道）：
     描述"这次查询（射线/扫掠）是什么类型"
     例：Visibility / Camera / 自定义（子弹/伤害检测）

核心机制：
  每个 Component 有一个 Object Type（对象通道）
  每个 Component 对每个通道有一个 Response（响应）
  碰撞/重叠是否发生 = 双方 Response 的组合判断
```

---

## 二、内置通道列表

```
Object Channels（对象通道，ECC_* 枚举）：

  ECC_WorldStatic    (0)  ← 静态场景几何（地形/建筑）
  ECC_WorldDynamic   (1)  ← 动态场景对象（可移动的道具）
  ECC_Pawn           (2)  ← 角色/NPC
  ECC_Visibility     (3)  ← 可见性检测（Trace 用）
  ECC_Camera         (4)  ← 摄像机检测（Trace 用）
  ECC_PhysicsBody    (5)  ← 纯物理对象（Simulate Physics 的物体）
  ECC_Vehicle        (6)  ← 车辆
  ECC_Destructible   (7)  ← 可破坏物体（Chaos Fracture）

  自定义通道：ECC_GameTraceChannel1 ~ ECC_GameTraceChannel18
  → 在 Project Settings → Collision 中配置名称

位掩码内部表示：
  每个通道是 uint32 的一位
  ECC_WorldStatic = bit 0
  ECC_WorldDynamic = bit 1
  ...
  → 碰撞过滤用位运算，速度极快
```

---

## 三、Response 类型

```
每个 Component 对每个通道有三种响应：

Ignore（忽略）：
  完全无视该通道的对象
  不产生 Hit/Overlap 事件
  不阻挡，直接穿过
  性能最优（BroadPhase 直接过滤）

Overlap（重叠）：
  可以穿过（不阻挡）
  但产生 BeginOverlap / EndOverlap 事件
  用于触发区域、拾取检测、伤害判定范围

Block（阻挡）：
  物理阻挡（不能穿过）
  产生 OnComponentHit 事件
  双方必须都是 Block 才能真正阻挡！
  
  关键规则：
    A.Block(B) && B.Block(A) → 真正阻挡（产生碰撞响应+Hit事件）
    A.Block(B) && B.Overlap(A) → 只产生 Overlap 事件，不物理阻挡
    A.Ignore(B) || B.Ignore(A) → 完全忽略
```

---

## 四、双方响应组合规则

```
碰撞响应决策表：

Object A Response | Object B Response | 最终结果
─────────────────────────────────────────────────
Block             | Block             | 物理阻挡 + Hit事件
Block             | Overlap           | Overlap 事件（不阻挡）
Block             | Ignore            | 完全忽略
Overlap           | Block             | Overlap 事件（不阻挡）
Overlap           | Overlap           | Overlap 事件（不阻挡）
Overlap           | Ignore            | 完全忽略
Ignore            | Block             | 完全忽略
Ignore            | Overlap           | 完全忽略
Ignore            | Ignore            | 完全忽略

规则总结：
  1. 只要有一方是 Ignore → 完全忽略
  2. 双方都是 Block → 物理阻挡（Hit）
  3. 一方 Block + 一方 Overlap → Overlap（不阻挡）
  4. 双方 Overlap → Overlap（不阻挡）

实际案例：
  子弹（PhysicsBody）打角色（Pawn）：
    子弹对 Pawn 通道：Block
    角色对 PhysicsBody 通道：Block
    → 子弹被角色阻挡，产生 Hit 事件

  触发区域（PhysicsBody）检测角色（Pawn）：
    触发区域对 Pawn 通道：Overlap
    角色对 PhysicsBody 通道：Block
    → 触发器产生 BeginOverlap（不阻挡角色行走）
```

---

## 五、自定义碰撞通道

```
创建自定义通道（Project Settings → Collision → Object Channels）：

步骤：
  1. Edit → Project Settings → Engine → Collision
  2. Object Channels → "New Object Channel"
  3. 设置名称（如 "Bullet"）
  4. 设置默认响应（Block / Overlap / Ignore）
  5. 保存 → 生成 ECC_GameTraceChannel1（或下一个可用编号）

C++ 中使用：
  // 在 DefaultEngine.ini 配置后，可用别名
  // +DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,
  //   DefaultResponse=ECR_Block,bTraceType=False,bStaticObject=False,Name="Bullet")

  // 代码中用枚举
  ECollisionChannel BulletChannel = ECC_GameTraceChannel1;
  // 或用字符串查找
  ECollisionChannel Ch = UEngineTypes::ConvertToCollisionChannel(
      UCollisionProfile::Get()->ConvertToCollisionChannel(true, "Bullet"));

Trace Channels（追踪专用通道）：
  不描述对象类型，只用于射线/扫掠查询
  Visibility：默认视觉可见性检测
  Camera：摄像机避障检测
  自定义：如 BulletTrace（子弹穿透检测）、DamageTrace（伤害检测）
  
  Trace Channel vs Object Channel 选择：
    追踪用途（LineTrace）→ Trace Channel
    对象类型（Simulate Physics/Overlap）→ Object Channel
```

---

## 六、DefaultEngine.ini 碰撞配置示例

```ini
[/Script/Engine.CollisionProfile]
; 自定义 Object Channel
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Block,
    bTraceType=False,bStaticObject=False,Name="Projectile")

; 自定义 Trace Channel
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel2,DefaultResponse=ECR_Block,
    bTraceType=True,bStaticObject=False,Name="WeaponTrace")

; 自定义 Collision Profile
+Profiles=(Name="Projectile",CollisionEnabled=ECollisionEnabled::QueryAndPhysics,
    ObjectTypeName="Projectile",
    CustomResponses=(
        (Channel="WorldStatic",Response=ECR_Block),
        (Channel="WorldDynamic",Response=ECR_Block),
        (Channel="Pawn",Response=ECR_Block),
        (Channel="PhysicsBody",Response=ECR_Block),
        (Channel="Projectile",Response=ECR_Ignore)
    ))
```

---

## 七、延伸阅读

- 📄 [2.3 碰撞响应详解](./03-collision-response.md)
- 📄 [2.4 Collision Profile 设计](./04-collision-profiles.md)
- 🔗 [Collision Channels 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-response-reference-in-unreal-engine)
