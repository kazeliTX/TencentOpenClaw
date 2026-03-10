# 3.4 物理约束总览

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 40 分钟

---

## 一、物理约束（Physics Constraint）概念

```
物理约束 = 限制两个刚体之间相对运动的"规则"

没有约束：两个刚体自由运动（6 DOF 各自）
有约束后：部分 DOF 被限制或完全锁定

类型（按限制的 DOF 分类）：
  Fixed（固定）：       锁定全部 6 DOF（相当于焊接在一起）
  Hinge（铰链）：       锁定 5 DOF，只允许绕一轴旋转（门/轮子）
  Prismatic（棱柱）：   锁定 5 DOF，只允许沿一轴平移（活塞/电梯）
  Ball-Socket（球窝）： 锁定 3 平移 DOF，允许 3 旋转 DOF（肩关节）
  Spring（弹簧）：      在某 DOF 上施加弹力（非锁定，有弹性）
  Custom（自定义）：    混合多种限制

UE 中的约束系统：
  UPhysicsConstraintComponent：蓝图/编辑器可用的约束 Actor
  FConstraintInstance：C++ 底层约束数据
  Physics Asset 中的约束（关节）：专用于骨骼物理（Ragdoll）
```

---

## 二、UPhysicsConstraintComponent 参数

```
Linear Motion（线性运动）：
  Linear X/Y/Z Motion：
    Free（自由）：该轴平移不受限
    Limited（有限）：该轴平移在范围内（设置 Linear Limit Size）
    Locked（锁定）：该轴平移完全禁止

  Linear Limit Size：两组件在 Limited 轴上最大位移（cm）

Angular Motion（角运动）：
  Angular Swing 1 / Swing 2 Motion：（X/Y 平面摆动）
    Free / Limited / Locked
  Angular Twist Motion：（绕 Z 轴扭转）
    Free / Limited / Locked
  
  Swing 1/2 Limit Angle：摆动角度上限（度）
  Twist Limit：扭转角度上限（度）

Linear / Angular Drive（驱动器）：
  启用驱动器 → 约束自动向目标位置/旋转施力
  Position Strength：弹簧刚度（越大越快到达目标）
  Velocity Strength：阻尼（越大越稳定/越不振荡）
  Maximum Force：驱动力上限
  Target Position / Target Velocity：目标位置/速度

Breakable（可断裂）：
  Linear Breakable Force：超过此力则断裂约束（如拉断绳子）
  Angular Breakable Torque：超过此力矩则断裂
```

---

## 三、约束坐标系

```
约束有自己的局部坐标系（Constraint Frame）：

Component 1 Frame：第一个组件的约束坐标系
Component 2 Frame：第二个组件的约束坐标系

铰链轴的设置：
  铰链 = 只允许绕某轴旋转
  该轴 = Twist Axis（扭转轴，默认 X 轴）
  → 设置 Angular Twist Motion = Free（扭转自由）
    Angular Swing 1/2 Motion = Locked（摆动锁定）
  → 物体只能绕 Constraint X 轴旋转

重要！：
  约束轴对应的是约束自身的坐标系，不是世界坐标
  需要在 Component 上设置约束的朝向
  通常：把约束 Component 的 X 轴对准旋转轴方向

常见错误：
  门的铰链轴方向不对 → 门绕错误轴旋转
  修复：旋转 PhysicsConstraintComponent 的 X 轴对准铰链方向（通常是 Z 轴/上方）
```

---

## 四、约束与 Physics Asset

```
PhysicsAsset 中的约束（关节）：
  每对父子骨骼之间可以有一个约束
  控制骨骼在 Ragdoll 中的运动范围
  
  与 UPhysicsConstraintComponent 的区别：
    UPhysicsConstraintComponent：独立 Actor/Component，连接两个任意刚体
    PhysicsAsset 约束：嵌入在骨骼物理资产中，自动对应骨骼父子关系
  
  共享底层：都使用 FConstraintInstance

PhysicsAsset 约束典型配置（角色手臂）：
  upperarm_l → lowerarm_l（肘关节）
    Angular Twist Motion: Limited  (±90°)
    Angular Swing 1: Limited (±90°)
    Angular Swing 2: Limited (±45°)
  → 手臂可以弯曲，但不能超出生理范围
```

---

## 五、约束求解顺序与稳定性

```
Chaos 约束求解顺序（影响稳定性）：

  1. 碰撞接触约束（最高优先级）
  2. 关节/铰链约束
  3. 弹簧约束
  4. 位置限制约束
  5. 速度约束（阻尼）

约束数量对性能的影响：
  每个约束增加约束求解迭代负担
  复杂约束链（N个串联约束）→ 求解收敛慢
  
  优化：
    减少约束迭代次数（Project Settings → Physics → Joint Pair Iterations）
    合并多个约束为一个复合约束
    对静止的约束对象使用 Kinematic（不参与动态求解）

约束稳定性问题与修复：
  问题：关节抖动（振荡不收敛）
  原因：约束刚度过高 + 步长过大
  修复：
    1. 开启 Substepping
    2. 增大 Angular/Linear Damping
    3. 降低 Drive Position Strength（弹簧刚度）
    4. 增大 Drive Velocity Strength（阻尼）

  问题：约束体相互穿入
  原因：质量比过大 + 迭代次数不足
  修复：
    1. 增加 Joint Pair Iterations（4→8）
    2. 用质量覆盖减小质量比
    3. 增大 Linear Limit Size（给予更多缓冲空间）
```

---

## 六、延伸阅读

- 📄 [3.5 铰链约束](./05-hinge-constraint.md)
- 📄 [3.6 弹簧约束](./06-spring-constraint.md)
- 🔗 [Physics Constraints](https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-constraints-in-unreal-engine)
