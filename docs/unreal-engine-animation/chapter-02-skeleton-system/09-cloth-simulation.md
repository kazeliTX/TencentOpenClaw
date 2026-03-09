# 2.9 Chaos 布料模拟

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、Chaos Cloth 概述

UE5 使用 **Chaos Cloth** 替代了 UE4 的 Apex Cloth，提供更精确的布料物理模拟：

```
Chaos Cloth vs Apex Cloth：

特性              Apex Cloth（UE4）    Chaos Cloth（UE5）
────────────────────────────────────────────────────────
引擎              PhysX 依赖           Chaos 物理，原生 UE5
GPU 加速          有限                  原生 GPU 加速
大规模布料        受限                  支持（如战场旗帜）
自碰撞            基础支持              完整支持
约束系统          简单                  完整（XPBD 求解器）
角色碰撞          胶囊体                支持 Physics Asset
风力场            有限                  完整 Wind System 集成
导入方式          Apex .apx 文件        直接在 UE 编辑器创建
```

---

## 二、布料设置流程

### 2.1 在编辑器中创建布料

```
步骤：
1. 打开 Skeletal Mesh 编辑器（双击 SK_xxx）
2. 工具栏 → Clothing（布料工具）
3. Section Selection Mode → 选择要添加布料的网格区域
   （如：披风、裙摆、斗篷对应的三角面）
4. Clothing → Create Clothing Data from Selection
   命名：Cloth_Cape（布料资产名）
5. 生成 Cloth Asset 后，调整参数

布料编辑器面板（在 Clothing 模式下）：
  • 蓝色顶点 = 固定顶点（绑定到骨骼，不受物理影响）
  • 黄色顶点 = 模拟顶点（受物理影响）
  • 权重值  0 = 完全固定，1 = 完全自由
```

### 2.2 布料权重绘制

```
权重绘制规则：

  [骨骼绑定区域]     [过渡区域]      [自由悬挂区域]
  权重 0.0 ──────── 0.0~0.5 ─────── 1.0
  完全固定           渐变            完全自由运动

示例：披风布料
  • 肩部附近：权重 0.0（绑定肩骨，随肩骨运动）
  • 中部：权重 0.3-0.6（过渡区）
  • 下摆：权重 1.0（完全自由摆动）
```

---

## 三、关键布料参数

```
Chaos Cloth 主要参数：

【刚度（Stiffness）】
  Vertical/Horizontal/Bend:  0.0~1.0
  值越高 = 布料越硬（0=完全软, 1=接近刚体）
  推荐：薄布料 0.1-0.3，厚布料 0.5-0.8

【阻尼（Damping）】
  Linear Damping:   0.01（平移阻尼，减少抖动）
  Angular Damping:  0.01（旋转阻尼）
  推荐：0.01-0.05，过大会让布料看起来"在水里"

【重力（Gravity）】
  Gravity Scale: 1.0（相对于世界重力的倍数）
  布料独立缩放：0.5 = 更轻的飘逸感

【碰撞（Collision）】
  Self Collision:         ✅（布料自身碰撞）
  Self Collision Thickness: 1.0cm
  Collision Thickness:    2.0cm（与角色碰撞的厚度）
  
【风力（Wind）】
  Wind Method:      Accurate（使用风速场）或 Legacy
  Drag Coefficient: 0.5（空气阻力系数）
  Lift Coefficient: 0.1（升力系数）
```

---

## 四、与骨骼的碰撞设置

```
布料需要与角色的 Physics Asset 形体碰撞：

1. 确保角色的 Physics Asset 中有碰撞形体
   （胸部、上臂等布料会接触的部位）

2. Skeletal Mesh 设置：
   Details → Clothing → Enable Clothing Simulation: ✅
   Physics Asset: 指定角色的 Physics Asset

3. 调整碰撞形体：
   PhAT 编辑器 → 适当扩大躯干/手臂胶囊体
   确保布料不会穿入身体
   
4. 运行时参数：
   GetMesh()->SetEnableGravity(true);        // 确保重力开启
   GetMesh()->SetWindActivated(true);        // 风力（如需要）
```

---

## 五、性能优化

```cpp
// 布料性能配置

// 1. LOD 距离控制（远处禁用布料模拟）
GetMesh()->ClothMaxDistanceScale = 1.0f;   // 完整模拟
// 在 LOD 设置中：
//   LOD0: 布料完整模拟
//   LOD1: 布料简化
//   LOD2+: 禁用布料（冻结最后姿势）

// 2. 暂停布料（如：角色不可见时）
GetMesh()->SuspendClothingSimulation();    // 暂停
GetMesh()->ResumeClothingSimulation();     // 恢复

// 3. 重置布料（如：角色传送后）
GetMesh()->ResetClothTeleportMode =
    EClothingTeleportMode::TeleportAndReset;  // 传送时重置布料位置

// 4. 布料子步数（影响精度 vs 性能）
// 项目设置 → Physics → Chaos Physics → Cloth Solver Iterations
// 默认 5，降低到 3 可提升性能但降低精度

// 5. 全局禁用布料（性能紧张时）
GetMesh()->bNoSkeletonUpdate = false;  // 不要设这个
// 使用 CVars：
// r.ClothPhysics 0  关闭所有布料模拟
// r.ClothPhysics 1  开启（默认）
```

---

## 六、常见问题

| 问题 | 原因 | 解决方案 |
|------|------|--------|
| 布料穿透身体 | 碰撞厚度不足 | 增大 Collision Thickness |
| 布料震颤抖动 | 阻尼太低或刚度不稳定 | 增大 Damping，降低 Stiffness |
| 传送后布料乱飞 | 大位移导致模拟爆炸 | 设置 ResetClothTeleportMode |
| 布料不随风飘动 | 风力未配置 | 检查 Wind Activation 和 Drag 系数 |
| 远处布料 LOD 突变 | LOD 切换太突然 | 调整 LOD 距离阈值，增加过渡范围 |

---

## 七、延伸阅读

- 🔗 [Chaos Cloth 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/clothing-simulation-in-unreal-engine)
- 🔗 [布料工具指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/clothing-tool-in-unreal-engine)
- 🔗 [Chaos Physics 概述](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-physics-in-unreal-engine)
