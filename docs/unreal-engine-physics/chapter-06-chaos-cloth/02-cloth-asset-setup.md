# 6.2 Cloth Asset 配置流程

> **难度**：⭐⭐⭐⭐☆

## 一、创建 Cloth Asset

```
流程（UE5 Chaos Cloth）：

Step 1：导入网格
  确保布料部分是独立的 Section（材质槽）
  例：裙摆 = Section 1，上衣 = Section 0（固定）
  
  布料网格建议：
    顶点数：1000-5000（性能/质量平衡）
    避免孤立顶点/非流形拓扑
    UV 展开合理（自碰撞厚度基于此计算）

Step 2：创建 Cloth Asset
  在 SkeletalMesh 编辑器中：
    Clothing → Add Clothing Asset（Chaos）
    → 创建 .cloth 文件

Step 3：分配布料 Section
  选择要成为布料的 Section（如 Section 1）
  Clothing → Apply Clothing Asset → 选择刚创建的 .cloth
  → Section 1 现在由 Chaos Cloth 驱动

Step 4：配置布料参数（在 Chaos Cloth 编辑器中）
  双击 .cloth 文件
  或：SkeletalMesh 编辑器 → Clothing 标签 → Open in Cloth Editor
```

## 二、Chaos Cloth 编辑器界面

```
主要区域：
  Viewport（左）：预览布料模拟
  Properties（右上）：全局参数面板
  Outliner（右下）：布料对象树

关键参数面板（Properties → Simulation）：
  Gravity Scale     ：重力缩放（1.0=正常，0.3=飘逸感）
  Animation Weight  ：与骨骼动画混合权重（0=纯物理，1=纯动画）
  
  Damping：
    Linear Drag     ：空气阻力（越大越慢，0.01-0.05）
    Angular Drag    ：旋转空气阻力
    
  Mass：
    Uniform Mass    ：均匀质量（kg，轻=飘逸，重=沉重）
    
  Constraint Stiffness：
    Stretch Stiffness   ：拉伸刚度（0-1）
    Bend Stiffness      ：弯曲刚度（0-1）
    Area Stiffness      ：面积保持（0-1，防止布料整体缩水）
```

## 三、权重绘制（基础）

```
进入 Paint 模式：
  Chaos Cloth 编辑器 → Tools → Paint

主要权重类型：
  Max Distance      ：顶点最大移动距离（0=固定，越大越自由）
  Backstop Distance ：背部止动距离（防止穿透）
  Backstop Radius   ：背部止动球半径

典型裙摆权重设置：
  裙摆腰部：Max Distance = 0（固定在角色腰部）
  裙摆中部：Max Distance = 5-20cm（逐渐过渡）
  裙摆下摆：Max Distance = 30-50cm（最大自由度）
  
  过渡要平滑（用软笔刷渐变，避免硬边界）
```

## 四、延伸阅读

- 📄 [6.3 布料参数详解](./03-cloth-parameters.md)
- 📄 [6.6 权重绘制详解](./06-painted-weights.md)
