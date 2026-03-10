# 2.8 复杂碰撞与优化

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 35 分钟

---

## 一、Complex Collision 使用场景

```
Complex Collision（三角网格碰撞）：

适合场景：
  子弹弹孔位置（需要精确命中点）
  脚步 IK（需要精确地面法线）
  地形射线检测
  复杂建筑内部结构检测

不适合场景：
  物理模拟（Simulate Physics 不支持 Complex Collision！）
  高频查询（每帧大量 Trace）
  移动对象的碰撞（性能差）

启用方式：
  Static Mesh → Details → Collision Complexity：
    Default（Simple and Complex）← 同时生成 Simple+Complex
    UseComplexAsSimple           ← 用 Complex 作为 Simple（仅 Query）
    UseSimpleAsComplex           ← 用 Simple 作为 Complex
    UseComplexAsSimpleAndComplex ← 全用 Complex（不推荐）

  LineTrace 中使用 Complex：
    Params.bTraceComplex = true;

性能数据对比（100 个随机 LineTrace，同一场景）：
  Simple Collision（凸包）：  约 0.1ms
  Complex Collision（三角网）：约 1.2ms（12倍！）
  → 非必要不用 Complex Collision
```

---

## 二、碰撞体优化技巧

```
技巧 1：减少凸包数量
  默认自动生成可能产生过多凸包（20~50个）
  手动调整：Static Mesh Editor → Collision → Auto Convex Collision
    Max Hull Verts：减少到 16~32（精度足够且性能更好）
    Hull Count：根据形状，3~6 个通常足够
  
技巧 2：用代理碰撞体（LOD Collision）
  高精度 Mesh 用于渲染
  低精度 Mesh 用于碰撞
  → 在 Static Mesh Editor 中为碰撞单独建模（低多边形盒/胶囊组合）

技巧 3：碰撞形状与视觉 Mesh 分离
  游戏中常见：子弹命中盒（Hit Box）比实际角色 Mesh 稍大
  原因：覆盖视觉边缘，命中感更好（"手感"）
  实现：单独的 BoxComponent/CapsuleComponent 作为 HitBox

技巧 4：距离裁剪碰撞
  远处对象禁用复杂碰撞：
  if (FVector::Distance(PlayerPos, ActorPos) > 3000.f)
      Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

技巧 5：Overlap 事件的批处理
  不要每帧查询所有 Overlap
  用事件（Begin/End）维护集合，比每帧 GetOverlappingActors 快
```

---

## 三、碰撞网格体积优化

```
对于大型场景（地形、建筑）：

分块碰撞：
  大型地形分割成 N×N 小块
  只激活玩家附近区域的碰撞
  远处区域：NoCollision 或 QueryOnly

Landscape 碰撞：
  UE Landscape 使用专用的高度场碰撞
  比三角网格快得多（利用高度场的规则性）
  Landscape 不需要手动设置碰撞

BSP vs Static Mesh 碰撞：
  BSP：已弃用（UE5），碰撞检测较慢
  Static Mesh：推荐，碰撞可以精确控制

碰撞 LOD：
  UE5 支持 Collision LOD（不同 LOD 层级用不同碰撞精度）
  远处 LOD：简单盒形
  近处 LOD：详细凸包
  → LOD 过渡时碰撞精度也自动降级
  
  设置：Static Mesh Editor → LOD Settings → 每个 LOD 的 Collision
```

---

## 四、World Partition 与碰撞流

```
UE5 World Partition（大世界分区）对碰撞的影响：

Streaming Level 未加载 → 该区域无碰撞！
  → 角色可能"掉穿地板"（地面 Mesh 未加载）
  
解决方案：
  1. 碰撞与视觉分离加载：
     视觉 Mesh 可以晚加载
     碰撞 Mesh 要优先加载（先于视觉可见距离）
  
  2. 设置 Collision Loading Range > Visual Loading Range：
     World Partition Cell → Collision Loading Distance 设更大
  
  3. 对关键地面 Actor 标记 Always Loaded：
     Details → World Partition → Is Spatially Loaded = false
```

---

## 五、延伸阅读

- 📄 [2.9 多碰撞体组合](./09-multi-body-collision.md)
- 🔗 [Collision Optimization](https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-in-unreal-engine)
- 🔗 [World Partition](https://dev.epicgames.com/documentation/en-us/unreal-engine/world-partition-in-unreal-engine)
