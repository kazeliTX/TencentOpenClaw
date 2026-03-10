# 9.2 Chaos 软体物体配置

> **难度**：⭐⭐⭐⭐⭐

## 一、Chaos Soft Body（UE5.2+ 实验性）

```
启用步骤：
  Edit → Plugins → Chaos Soft Body ✓（实验性插件）
  重启编辑器

创建软体：
  1. 准备闭合 Mesh（不能有开口，UV 完整）
  2. 内容浏览器 → 右键 StaticMesh
     → Create → Chaos Soft Body Asset
  3. 在场景中放置 ChaosDeformableSolverActor
  4. 将 SoftBody Asset 拖入场景
     → 自动生成 ChaosDeformableCollectionActor

核心参数：
  Strain（应变刚度）：
    0.1 → 极软（果冻感）
    0.5 → 中等软包
    0.9 → 略微可变形的硬体
    
  Damping：
    0.01~0.1 → 越大越快稳定
    
  Gravity Scale：重力缩放（0 = 漂浮，-1 = 反重力）
  
  Collision Type：
    Surface Particles → 表面粒子碰撞（较精确）
    None → 无碰撞（纯装饰变形）

注意：Chaos Soft Body 在 UE5.3 仍为实验性，生产环境慎用
  → 稳定软体方案仍推荐用 Chaos Cloth（薄软包近似）
```

## 二、延伸阅读

- 📄 [9.3 UCableComponent](./03-cable-component.md)
