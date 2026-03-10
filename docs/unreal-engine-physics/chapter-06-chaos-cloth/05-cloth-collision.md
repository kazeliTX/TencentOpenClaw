# 6.5 布料碰撞（自碰撞/环境碰撞）

> **难度**：⭐⭐⭐⭐⭐

## 一、布料与环境碰撞

```
Chaos Cloth 与环境碰撞类型：

1. Skeletal Body 碰撞（主要）
   布料与自身骨骼的 Physics Asset Body 碰撞
   → 防止裙摆穿进腿部
   设置：Physics Asset 中对应骨骼的 Body 开启碰撞
   
   最重要的 Body（角色穿裙）：
     thigh_l / thigh_r（大腿）
     pelvis（骨盆）
     spine_01（腰部）
     
2. Convex Collision（凸体碰撞）
   在 Chaos Cloth 编辑器中添加的额外碰撞体
   → 更精确的角色体型碰撞
   
3. 环境静态碰撞
   布料与静态网格（墙/地面）碰撞
   需要 StaticMesh 开启 "Generate Overlap Events"
   布料的碰撞半径（Cloth Collision Margin）决定接触距离
```

## 二、自碰撞（Self Collision）

```
控制布料与自身的穿插：

开启方式：
  Chaos Cloth Editor → Simulation → Self Collision:
    Self Collision Thickness：自碰撞粒子球半径（cm）
      太小 → 自碰撞无效，布料穿透自身
      太大 → 布料无法自然折叠（像充气感）
      推荐：0.5~2.0 cm（布料厚度的 1/2~1 倍）

  Self Collision Stiffness：自碰撞约束刚度（0~1）
    0.3~0.7 为典型值
    太高 → 布料反弹剧烈（弹珠感）
    太低 → 自碰撞响应延迟，仍有穿插

性能警告：
  自碰撞是 Chaos Cloth 中最贵的功能
  每帧需要 O(n²) 的粒子对检测（使用 SpatialHash 加速）
  建议：
    仅对必要布料开启自碰撞（裙摆）
    减少布料顶点数（≤2000）
    远距离 LOD 关闭自碰撞
```

## 三、碰撞调试

```
调试命令（PIE 中）：
  p.ChaosCloth.DebugDraw.CollisionParticles 1
  → 显示碰撞粒子（绿色球体）
  
  p.ChaosCloth.DebugDraw.SelfCollision 1
  → 显示自碰撞约束（红色）
  
  p.ChaosCloth.DebugDraw.PhysMeshWireframe 1
  → 显示布料网格线框

常见穿插修复思路：
  穿进大腿 → 增大 thigh Body 半径 + 调整位置
  裙摆穿地面 → 增大 Cloth Collision Margin
  布料穿过自身 → 增大 Self Collision Thickness
  布料粘在身体上 → 检查 Max Distance 权重图，确保足够自由度
```

## 四、延伸阅读

- 📄 [6.6 权重绘制](./06-painted-weights.md)
