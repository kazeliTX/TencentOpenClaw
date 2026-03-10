# 5.1 Physics Asset 总览与 PhAT 工具

> **难度**：⭐⭐⭐☆☆

## 一、什么是 Physics Asset

```
Physics Asset（.phat）：
  与 SkeletalMesh 绑定的物理资产
  定义骨骼网格的碰撞体（Bodies）和关节约束（Constraints）
  
  用途：
    布娃娃（Ragdoll）—— 最主要用途
    命中反应（Hit Reaction）
    角色与世界的物理交互（推箱子、游泳阻力）
    动画物理混合

  文件关系：
    SkeletalMesh → 引用 PhysicsAsset
    PhysicsAsset → 包含 Bodies[] + Constraints[]
    Body → 对应某根骨骼（如 "spine_01"）
    Constraint → 连接父骨骼 Body 和子骨骼 Body
```

## 二、PhAT 工具界面

```
打开方式：
  双击 .phat 文件 → 打开 Physics Asset Editor（即 PhAT）

主要面板：
  Skeleton Tree（左）：骨骼层级，显示哪些骨骼有 Body
  Viewport（中）：实时预览，绿色=Body，蓝色=Constraint
  Details（右）：选中 Body/Constraint 的属性

工具栏关键按钮：
  Simulation：开启实时物理模拟预览（可施力测试）
  New Body：为选中骨骼创建 Body
  Delete Body：删除选中 Body（及其 Constraint）
  
骨骼颜色含义：
  绿色 Body：正常
  黄色 Body：选中状态
  灰色骨骼：无 Body（不参与物理）
  蓝色线：Constraint
```

## 三、自动生成 vs 手动配置

```
自动生成（推荐起点）：
  PhAT → Tools → Generate All Bodies
  → 为所有骨骼自动创建胶囊/球体 Body
  → 自动连接父子 Body 创建 Constraint
  
  缺点：自动生成的 Body 尺寸不精确，需逐一手动调整

典型人形角色骨骼 Body 分配：
  骨骼              Body 类型    近似尺寸
  pelvis            胶囊         R=15 H=25
  spine_01          胶囊         R=15 H=20
  spine_02          胶囊         R=14 H=18
  spine_03          胶囊         R=13 H=16
  clavicle_l/r      无 Body      （太小，跳过）
  upperarm_l/r      胶囊         R=8  H=25
  lowerarm_l/r      胶囊         R=7  H=22
  hand_l/r          球体         R=6
  neck_01           胶囊         R=8  H=10
  head              球体/箱体    R=12 或 15×18×15
  thigh_l/r         胶囊         R=10 H=35
  calf_l/r          胶囊         R=8  H=30
  foot_l/r          箱体         12×8×20

性能建议：
  人形角色建议 15-20 个 Body（不要超过 26 个）
  手指/脚趾骨骼通常跳过（价值/性能比低）
  LOD 距离 > 10m 可只保留 8-10 个主躯干 Body
```

## 四、延伸阅读

- 📄 [5.2 Body 设置详解](./02-body-setup.md)
- 🔗 [Physics Asset Editor](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-asset-editor-in-unreal-engine)
