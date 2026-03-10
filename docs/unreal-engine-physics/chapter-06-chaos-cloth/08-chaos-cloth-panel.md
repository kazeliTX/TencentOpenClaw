# 6.8 Chaos Cloth 编辑器详解

> **难度**：⭐⭐⭐⭐☆

## 一、编辑器工具栏

```
顶部工具栏：
  ▶ Simulate：开启/停止实时模拟预览
  ⚡ Reset：重置布料到初始位置
  ☁ Wind：开启风场预览（用 WindDirectionalSource 或内置风力）
  
  视图模式切换：
    Wireframe  ：显示布料网格线框
    Visualization → Max Distance：热力图显示权重值
    Visualization → Normals：显示法线（检查面朝向）

模拟控件（Simulation 面板）：
  Timestep        ：子步时间（秒）
  Num Substeps    ：每帧子步数
  Max Iterations  ：约束迭代次数
  Gravity         ：重力向量（可以改变重力方向模拟零重力）
```

## 二、Simulation 完整参数列表

```
Mass:
  Uniform Mass            ：均匀质量（kg）
  Use Gravity Override    ：局部重力覆盖
  Gravity Override        ：自定义重力向量

Damping:
  Damping Coefficient     ：线性阻尼
  Local Damping Coefficient：局部速度阻尼（减少振荡）
  
Long Range Constraints:
  Tether Stiffness        ：系绳刚度（防止过度偏移）
  Tether Scale            ：系绳长度缩放
  
Aerodynamics（空气动力）:
  Air Drag Coefficient    ：空气阻力系数（越大越容易被风吹）
  Air Lift Coefficient    ：升力系数（让布料向上飘）
  Air Fluid Density       ：空气密度（影响阻力大小）

Collisions:
  Collision Thickness     ：碰撞球半径（cm）
  Friction Coefficient    ：布料摩擦系数
  Self Collision Thickness：自碰撞球半径
  Self Collision Stiffness：自碰撞刚度
  
Constraints:
  Stretch Stiffness       ：拉伸刚度
  Stretch Damping         ：拉伸阻尼
  Bend Stiffness          ：弯曲刚度
  Bend Damping            ：弯曲阻尼
  Shear Stiffness         ：剪切刚度
  Area Stiffness          ：面积保持刚度
```

## 三、快捷调参技巧

```
布料"果冻感"（参数过软）修复：
  ↑ Stretch Stiffness（拉伸变硬）
  ↑ Bend Stiffness   （弯曲变硬）
  ↑ Damping          （减少振荡）

布料"太硬/纸板感"修复：
  ↓ Bend Stiffness
  ↓ Area Stiffness

布料"持续抖动不停"修复：
  ↑ Damping Coefficient
  ↑ Local Damping Coefficient
  ↑ Sleep Threshold（让布料更容易入睡）

布料被风吹时感觉太重：
  ↓ Uniform Mass
  ↑ Air Drag Coefficient
  ↑ Air Lift Coefficient
```

## 四、延伸阅读

- 📄 [6.9 运行时布料控制](./09-runtime-cloth-control.md)
