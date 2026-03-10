# 6.1 Chaos Cloth 总览与架构

> **难度**：⭐⭐⭐☆☆

## 一、从 NvCloth 到 Chaos Cloth

```
UE4 旧布料系统（NvCloth）：
  基于 NVIDIA 的 NvCloth 库
  在 UE5 中已废弃（但仍可用）
  
Chaos Cloth（UE5+）：
  Epic 自研，基于 Chaos 物理框架
  完全 CPU 侧运算（UE5.0-5.2）
  GPU 加速布料（UE5.3+ 实验性）
  XPBD（Extended Position-Based Dynamics）求解器
  
主要改进：
  更真实的拉伸/弯曲约束
  更好的自碰撞处理
  原生多线程支持（Chaos Async Physics）
  更精细的参数控制（逐粒子属性）
```

## 二、Chaos Cloth 工作原理

```
布料网格 → 粒子系统：
  每个顶点 = 一个物理粒子（FClothParticle）
  粒子有：位置、速度、质量（0=固定，>0=自由）
  
约束系统（连接粒子）：
  拉伸约束（Stretch）：边长约束，防止过度拉伸
  弯曲约束（Bend）：二面角约束，控制布料硬度
  剪切约束（Shear）：对角边约束，防止菱形变形
  
求解流程（每帧）：
  1. 施加外力（重力/风力）
  2. 更新速度和预测位置
  3. 约束迭代求解（XPBD，多次迭代）
  4. 碰撞检测与响应
  5. 输出最终顶点位置 → 更新 SkeletalMesh
  
迭代次数影响：
  迭代 2 次：快但约束可能违反（布料会拉伸变形）
  迭代 8 次：慢但精确（推荐布料精细场景）
  迭代 16 次：极精确，用于过场动画
```

## 三、适用场景

```
Chaos Cloth 适合：
  ✅ 角色衣物（裙摆/斗篷/飘带）
  ✅ 旗帜/横幅
  ✅ 帐篷/窗帘
  ✅ 布料道具（桌布/抹布）
  
不适合（用其他方案）：
  ❌ 绳索（用物理约束链或 CableComponent）
  ❌ 橡皮泥变形（用软体物理）
  ❌ 纸张撕裂（需要自定义拓扑变化）
  ❌ 金属板折叠（用刚体约束）
```

## 四、延伸阅读

- 📄 [6.2 Cloth Asset 配置流程](./02-cloth-asset-setup.md)
- 🔗 [Chaos Cloth](https://dev.epicgames.com/documentation/en-us/unreal-engine/chaos-cloth-in-unreal-engine)
