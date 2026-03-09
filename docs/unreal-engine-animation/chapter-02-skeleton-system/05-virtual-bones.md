# 2.5 虚拟骨骼与辅助骨骼

> **难度**：⭐⭐☆☆☆ | **阅读时间**：约 20 分钟

---

## 一、虚拟骨骼（Virtual Bones）

### 1.1 什么是虚拟骨骼

虚拟骨骼（Virtual Bone）是 UE 在两根已有骨骼之间**动态插值**出来的骨骼，**不影响蒙皮**，仅作为 AnimGraph 中的参考点。

```
定义方式：
  虚拟骨骼 = 骨骼 A 和骨骼 B 之间插值出的中间点
  
  例：VB_MidPoint = Lerp(hand_l, shoulder_l, 0.5)
  
  这个虚拟骨骼在 AnimGraph 中可以像真实骨骼一样被引用，
  但不需要在 DCC 软件中预先创建。
```

### 1.2 创建虚拟骨骼

```
在 Skeleton Editor 中：
  工具栏 → Virtual Bones → Add Virtual Bone
  
  设置：
    Source Bone: 起始骨骼（如 hand_l）
    Target Bone: 目标骨骼（如 lowerarm_l）
    名称：VB_hand_lowerarm（自动生成 "VB_" 前缀）
```

### 1.3 用途

```
常见用途：

1. IK 中间目标点
   例：双手持枪时，虚拟骨骼在两手之间，
   作为 Two-Bone IK 的中间参考

2. 脊椎弯曲控制点
   在 spine_01 和 spine_03 之间创建虚拟骨骼
   作为 LookAt 节点的参考点

3. 动画重定向辅助
   在骨骼比例差异较大时，提供更精确的对齐参考

4. 程序化动画锚点
   作为粒子特效、挂载点的稳定参考
```

---

## 二、辅助骨骼（Twist Bones / Helper Bones）

辅助骨骼是实际存在于骨骼层级中的骨骼，由 Rigger 在 DCC 软件中预先创建，用于改善蒙皮质量。

### 2.1 Twist Bone（扭转骨骼）

```
问题：前臂扭转时，手腕附近出现"套索"扭曲效果（LBS 问题）

解决方案：在前臂和手腕之间添加 2-3 根扭转骨骼

骨骼层级：
  lowerarm_l
  ├── lowerarm_twist_01_l  （距手腕 66%，权重参与 33%）
  ├── lowerarm_twist_02_l  （距手腕 33%，权重参与 66%）
  └── hand_l

每根扭转骨骼旋转量：
  twist_01: 旋转 lowerarm_l 扭转量的 33%
  twist_02: 旋转 lowerarm_l 扭转量的 66%
  hand_l:   旋转 lowerarm_l 扭转量的 100%

效果：扭转量分散到多根骨骼，平滑过渡，避免单骨骼大角度变形

在 AnimGraph 中自动驱动 Twist Bone：
  使用 "Twist Corrective" 节点 或 Control Rig 中的 Twist Solver
```

### 2.2 Volume Bone（体积骨骼）

```
问题：手肘弯曲时，肘部内侧体积丢失（"糖纸效应"）

解决方案：在手肘弯曲轴处添加体积辅助骨骼

典型配置：
  upperarm_l
  ├── upperarm_bicep_l   （上臂外侧，随弯曲角度膨胀）
  ├── upperarm_inner_l   （上臂内侧，随弯曲角度收缩）
  └── lowerarm_l
      ├── lowerarm_elbow_l （肘部补偿骨骼）
      └── ...

在 AnimBP 中用 "Bone Driven Controller" 节点驱动：
  Source Bone: upperarm_l（读取弯曲角度）
  Source Axis: Pitch（弯曲轴）
  Target Bone: upperarm_bicep_l
  Target Axis: Scale X（沿骨骼方向缩放）
  Curve: 配置弯曲角度 → 缩放量的映射曲线
```

---

## 三、UE5 中配置骨骼驱动器

```
在 AnimBP 的 AnimGraph 中：

节点：Bone Driven Controller
      （在 AnimGraph 节点搜索中输入 "driven"）

参数设置：
  Source Bone: clavicle_l          （输入骨骼）
  Source Component: Rotation Y     （读取旋转 Y 轴分量）
  Multiplier: 0.5                  （缩放系数）
  Range Max: 90.0                  （输入最大值）
  Destination Bone: clavicle_helper_l （目标骨骼）
  Destination Component: Rotation Z   （修改旋转 Z 轴）
  Modification Mode: Add to Input     （叠加到现有值）
```

---

## 四、UE5 Mannequin 的辅助骨骼列表

UE5 标准 Mannequin 自带以下辅助骨骼：

| 骨骼名称 | 区域 | 作用 |
|---------|------|------|
| `lowerarm_twist_01_l/r` | 前臂 | 前臂扭转分散 |
| `upperarm_twist_01_l/r` | 上臂 | 上臂扭转分散 |
| `thigh_twist_01_l/r` | 大腿 | 大腿扭转分散 |
| `calf_twist_01_l/r` | 小腿 | 小腿扭转分散 |
| `ik_foot_l/r` | 脚 | IK 效应器 |
| `ik_hand_l/r` | 手 | IK 效应器 |
| `ik_foot_root` | 根 | 脚部 IK 根节点 |
| `ik_hand_root` | 根 | 手部 IK 根节点 |
| `ik_hand_gun` | 手 | 武器握持 IK |

---

## 五、延伸阅读

- 📄 [2.8 运行时骨骼变换修改](./08-bone-modification.md)
- 🔗 [Virtual Bones 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-bones-in-unreal-engine)
- 🔗 [Control Rig Twist Solver](https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-solvers-in-unreal-engine)
