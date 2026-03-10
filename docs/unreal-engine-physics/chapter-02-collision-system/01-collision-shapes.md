# 2.1 碰撞体类型

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 35 分钟

---

## 一、Simple Collision vs Complex Collision

```
Simple Collision（简单碰撞）：
  由基本几何体组成：球 / 胶囊 / 盒 / 凸包
  用于物理模拟（Simulate Physics）
  用于快速射线检测（bTraceComplex = false）
  性能高，精度中等

Complex Collision（复杂碰撞）：
  使用 Mesh 的三角形网格（每个三角面都参与碰撞）
  精度最高（完全贴合 Mesh 轮廓）
  不能用于 Simulate Physics（只能 Query Only）
  性能低，仅用于精确射线检测

选择原则：
  物理模拟 → 必须用 Simple Collision
  视觉精确射线（子弹着点、脚步IK） → Complex Collision
  大多数游戏对象 → Simple Collision 足够
```

---

## 二、基础碰撞形状类型

```
球形（Sphere）：
  参数：半径 R
  体积：4/3 * π * R³
  优点：碰撞检测最快（只需比较球心距离）
  适用：抛射物、硬币、爆炸范围、角色头部
  
  Static Mesh 中添加：
    Details → Collision → Add Sphere Simplified Collision

胶囊（Capsule）：
  参数：半径 R，半高 HalfHeight
  由两个半球 + 一段圆柱组成
  优点：旋转不变形（角色站立/蹲伏都用同一胶囊旋转）
  适用：角色、NPC、人形碰撞
  
  UCharacterMovementComponent 默认使用 UCapsuleComponent

盒（Box）：
  参数：三轴半扩展 HalfExtent(X, Y, Z)
  优点：AABB/OBB 检测快，视觉贴合方形物体
  适用：箱子、房间、门框、障碍物

凸包（Convex Hull）：
  包裹网格顶点的最小凸多边形
  自动生成（UE 内置凸包分解工具）
  精度介于盒形和三角网格之间
  适用：不规则形状（岩石、道具、武器）
  
  凸包数量上限：每个 Static Mesh 最多 255 个凸包
  → 凸包越多精度越高，性能越低
```

---

## 三、多碰撞体组合（Compound Collision）

```
一个 Static Mesh 可以有多个 Simple Collision 形状：

优势：
  用多个基本形状拼合复杂物体
  比单一凸包更快（分散为更小的凸包）
  允许部分区域不同碰撞属性

添加方式（Static Mesh Editor）：
  Collision 菜单 → Add Box/Sphere/Capsule/Sphyl Simplified Collision
  → 自动添加到现有碰撞形状集合
  → 可调整每个形状的位置/旋转/缩放

典型案例：L 形走廊
  用两个 Box 拼出 L 形
  比单一凸包更快，精度相同

典型案例：角色（带武器）
  躯体：胶囊
  头部：球
  武器：OBB（盒）
  → 组合3个形状，检测精度大幅提升

FBX 导入时自动生成碰撞：
  命名规则（在3D软件中预设）：
    UCX_MeshName   → 凸包碰撞
    UBX_MeshName   → 盒形碰撞
    USP_MeshName   → 球形碰撞
    UCP_MeshName   → 胶囊碰撞
  导入时 UE 识别前缀，自动转换为对应碰撞类型
```

---

## 四、碰撞形状性能对比

```
碰撞检测速度（由快到慢）：

  球 vs 球：      O(1)  ~0.001ms   只比较球心距离
  球 vs 胶囊：    O(1)  ~0.002ms   点对线段最近距离
  胶囊 vs 胶囊：  O(1)  ~0.003ms   线段对线段
  球 vs 盒：      O(1)  ~0.003ms
  盒 vs 盒(AABB): O(1)  ~0.002ms   轴分离定理
  盒 vs 盒(OBB):  O(n)  ~0.010ms   15 轴分离测试
  凸包 vs 凸包：  O(n)  ~0.010~0.05ms  GJK 算法
  凸包 vs 三角网：O(nm) ~0.1~1ms   每三角面
  三角网 vs 三角网：不支持（无法物理模拟）

实际建议：
  抛射物（每帧大量碰撞）：强制用球形
  角色：胶囊
  普通道具/场景：1~4个凸包
  精确射线（子弹穿入点）：复杂碰撞（只用于 Trace）
```

---

## 五、碰撞形状调试

```
Static Mesh Editor 中：
  Collision 菜单 → Show Simple Collision（绿色线框）
  Collision 菜单 → Show Complex Collision（蓝色网格）

运行时：
  show collision              ← 显示所有碰撞体
  show simplecollision        ← 只显示 Simple
  show complexcollision       ← 只显示 Complex

Chaos Debug Draw：
  p.Chaos.DebugDraw.ShowCollision 1

C++ 查询碰撞形状数量：
  FBodySetupAggGeom& AggGeom = StaticMesh->GetBodySetup()->AggGeom;
  int32 Spheres   = AggGeom.SphereElems.Num();
  int32 Boxes     = AggGeom.BoxElems.Num();
  int32 Capsules  = AggGeom.SphylElems.Num();
  int32 Convexes  = AggGeom.ConvexElems.Num();
```

---

## 六、延伸阅读

- 📄 [2.2 碰撞通道系统](./02-collision-channels.md)
- 🔗 [Simple vs Complex Collision](https://dev.epicgames.com/documentation/en-us/unreal-engine/simple-versus-complex-collision-in-unreal-engine)
- 🔗 [Collision Shape Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-in-unreal-engine)
