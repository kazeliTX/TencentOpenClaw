# 7.1 Physics Asset（PhAT）完全指南

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、Physics Asset 的作用

```
Physics Asset 是骨骼 Mesh 的物理表示，包含：
  1. Bodies（物理体）：每根骨骼对应的碰撞形状（Sphere/Box/Capsule/Convex）
  2. Constraints（约束）：关节之间的物理约束（限制旋转范围）

用途：
  ✅ 布娃娃（Ragdoll）物理模拟
  ✅ 碰撞检测（角色与环境）
  ✅ Physical Animation Component（物理驱动姿势）
  ✅ 受击反应（局部物理）
  ✅ Chaos 布料模拟的碰撞体
```

---

## 二、创建 Physics Asset

```
自动生成：
  骨骼 Mesh 导入时勾选 Create Physics Asset
  UE 自动为每根骨骼生成碰撞体
  
手动打开：
  内容浏览器 → 右键 SkeletalMesh → Physics → Create Physics Asset
  或直接双击已有 Physics Asset 打开 PhAT 编辑器
```

---

## 三、PhAT 编辑器

```
界面区域：
  ┌───────────────────────────────────────────┐
  │  骨骼树（左）     3D 视口      属性面板（右）│
  │                                            │
  │  Bodies 列表      角色预览     选中 Body/   │
  │  Constraints 列表              Constraint  │
  │                               的详细设置   │
  └───────────────────────────────────────────┘

视口模式：
  Bodies：显示所有碰撞体（不同颜色区分形状类型）
  Constraints：显示所有关节约束（锥形=旋转范围）
  Simulation：实时物理模拟预览（可推动角色测试布娃娃）
```

---

## 四、配置 Bodies（碰撞体）

```
推荐形状选择：
  头部：Sphere（球体，最快）
  躯干：Capsule 或 Box
  大腿/小腿/上臂/前臂：Capsule（最适合细长骨骼）
  脚部：Box（贴地更准确）
  手部：Sphere 或 Box

配置参数：
  Mass（质量）：
    head:    5kg
    torso:   30kg
    thigh:   10kg
    calf:    7kg
    foot:    2kg
    
  Linear Damping（线性阻尼）：0.01（布娃娃默认即可）
  Angular Damping（旋转阻尼）：0.01

  Collision Response：
    布娃娃模式：Block（与世界碰撞）
    受击动画模式：Overlap 或 Query Only（只查询不阻挡）
```

---

## 五、配置 Constraints（关节约束）

```
关节约束限制两个物理体之间的相对运动：

约束类型：
  Swing 1/2：摆动范围（如手臂左右/前后摆动的最大角度）
  Twist：扭转范围（如手腕旋转的最大角度）

人体关节典型配置：

关节          Swing1   Swing2   Twist    说明
────────────────────────────────────────────────
spine_01      ±15°     ±15°     ±10°    脊椎（小范围）
neck          ±30°     ±30°     ±20°    颈部（中等）
shoulder      ±90°     ±90°     ±45°    肩部（大范围）
elbow         ±5°      ±140°    ±5°     肘部（只能弯曲一个方向）
wrist         ±45°     ±45°     ±60°    手腕（中等+扭转）
hip           ±60°     ±45°     ±20°    髋关节（大范围）
knee          ±5°      ±130°    ±5°     膝盖（只能向后弯曲）
ankle         ±30°     ±45°     ±20°    踝关节
```

---

## 六、布娃娃配置最佳实践

```
1. Body 数量：不需要每根骨骼都有 Body
   手指：可以合并为 1~2 个 Body（而非每节指骨单独）
   面部骨骼：可以不加 Body（通常不需要物理）
   关键骨骼必须有 Body：pelvis、spine_01~03、head、四肢主骨

2. 物理稳定性检查（模拟模式）：
   点击 Simulate → 让角色自然倒下
   检查：是否有穿插（Body 互相嵌入）？
   调整：增大 Body 尺寸避免嵌入

3. Profile 系统（多套物理配置）：
   PhAT → Profiles → 添加多个 Profile
   例：
     Ragdoll Profile：所有 Body 开启物理，Constraint 宽松
     HitReaction Profile：只有上半身 Body 开启，Constraint 较紧
   → 运行时通过 ApplyPhysicsProfile() 切换
```

---

## 七、延伸阅读

- 📄 [7.2 布娃娃系统](./02-ragdoll-system.md)
- 🔗 [Physics Asset 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-asset-editor-in-unreal-engine)
