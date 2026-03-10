# 8.4 二级运动

> **难度**：⭐⭐⭐⭐☆

## 一、胸口/骨骼附件的二级运动

```
二级运动（Secondary Motion）：
  角色运动时，身体附件（首饰/头发/口袋）随主体运动产生滞后摆动
  
  实现方案：
    1. 物理动画（最精确但开销大）
    2. KMath（UE5 新增，基于弹簧算法，性能佳）
    3. AnimDynamics（AnimBP 节点，常用）
    4. Jiggle Bone（简化弹簧骨骼）

AnimDynamics（AnimBP 中）：
  AnimGraph 节点：AnimDynamics
  参数：
    Bound Bone：需要摆动的骨骼
    Box Extent：运动约束范围（限制摆动幅度）
    Angular Spring Constant：角弹簧（弹回力）
    Angular Damping：角阻尼（防止无限振荡）
    Linear Spring Constant：线弹簧（复位力）
    
    典型值（马尾辫）：
      Angular Spring = 80, Angular Damping = 20
      Box Extent = (5, 5, 3)

物理动画方案（更真实）：
  对头发/饰品骨骼单独建 PhysicsAsset Body
  设置独立的 PhysicalAnimationProfile
  OrientationStrength = 20（很弱，大量物理自由度）
  AngularVelocityStrength = 5（低阻尼，摆动明显）
```
