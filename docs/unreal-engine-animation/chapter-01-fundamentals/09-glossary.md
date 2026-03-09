# 1.9 术语速查表

> 按字母/拼音顺序排列，方便快速查找

---

## A

| 术语 | 英文 | 说明 |
|------|------|------|
| 瞄准偏移 | Aim Offset | 特殊的 Blend Space，用于上半身瞄准方向叠加 |
| 动画蓝图 | Animation Blueprint (AnimBP) | 可视化动画逻辑编辑器，每个角色类型对应一个 |
| 动画合成 | Animation Composite | 将多段 AnimSequence 按顺序拼接的资产 |
| 动画曲线 | Animation Curve | 时间轴上的浮点数据轨道，可驱动 Morph Target 等 |
| 动画实例 | Animation Instance (AnimInstance) | 运行时动画控制器，每个角色有独立实例 |
| 动画通知 | Anim Notify | 动画时间轴上的事件触发点（瞬时或持续段） |
| 动画序列 | Animation Sequence (AnimSeq) | 基础关键帧动画数据资产 |
| 动画插槽 | Animation Slot | AnimGraph 中 Montage 的插入点 |

## B

| 术语 | 英文 | 说明 |
|------|------|------|
| 基础姿势 | Base Pose | 作为混合/叠加基准的参考姿势 |
| 混合空间 | Blend Space | 根据输入参数混合多个动画的资产 |
| 骨骼 | Bone | 骨骼层级中的单个关节/节点 |
| 骨骼空间 | Bone Space | 相对于特定骨骼自身的坐标空间 |
| 布娃娃 | Ragdoll | 完全物理驱动的角色死亡/受击状态 |

## C

| 术语 | 英文 | 说明 |
|------|------|------|
| 缓存姿势 | Cache Pose / Cached Pose | AnimGraph 中复用已计算姿势的节点 |
| Control Rig | Control Rig | UE5 引擎内可视化绑定与动画制作系统 |
| 组件空间 | Component Space | 相对于 SkeletalMeshComponent 原点的坐标空间 |
| 压缩 | Compression | 减小动画数据文件大小的算法（默认 ACL）|

## D

| 术语 | 英文 | 说明 |
|------|------|------|
| 双四元数蒙皮 | Dual Quaternion Skinning (DQS) | 改进型蒙皮，解决关节扭曲问题 |
| DCC | DCC（Digital Content Creation）| Maya、Blender、3ds Max 等美术制作软件 |

## F

| 术语 | 英文 | 说明 |
|------|------|------|
| FABRIK | FABRIK | Forward And Backward Reaching IK，链式 IK 算法 |
| 全身 IK | Full Body IK (FBIK) | UE5 多目标全身 IK 系统（基于 XPBD）|
| 正向运动学 | Forward Kinematics (FK) | 从关节角度计算末端位置（父→子传递）|

## G

| 术语 | 英文 | 说明 |
|------|------|------|
| GPU 蒙皮 | GPU Skinning | 在 GPU 上执行骨骼矩阵应用到顶点的计算 |

## I

| 术语 | 英文 | 说明 |
|------|------|------|
| 逆向运动学 | Inverse Kinematics (IK) | 从末端目标位置反推关节角度（子→父求解）|
| IK 绑定 | IK Rig | UE5 中定义骨骼链和效应器的资产 |
| IK 重定向器 | IK Retargeter | UE5 中将动画从一个骨骼重定向到另一个的工具 |

## L

| 术语 | 英文 | 说明 |
|------|------|------|
| 分层混合 | Layered Blend Per Bone | 按骨骼分层的动画混合节点 |
| 线性混合蒙皮 | Linear Blend Skinning (LBS) | 传统蒙皮算法，多骨骼加权平均 |
| 链接动画层 | Linked Anim Layer | UE5 运行时可替换的动画逻辑层 |
| Live Link | Live Link | 从 DCC 实时流送动画数据到 UE 的系统 |
| LOD | Level of Detail | 细节层次，根据距离降低模型/动画精度 |
| 局部空间 | Local Space | 相对于父骨骼的坐标空间 |

## M

| 术语 | 英文 | 说明 |
|------|------|------|
| 主控姿势组件 | Master Pose Component | 多 Mesh 部件共享同一套骨骼计算的系统 |
| ML 变形器 | ML Deformer | UE5 机器学习驱动的网格变形系统 |
| 蒙太奇 | Montage | 可程序分段控制的动画资产 |
| 运动匹配 | Motion Matching | 从数据库中实时搜索最匹配动画姿势的技术 |
| 运动扭曲 | Motion Warping | 动态调整根运动使角色精确落位的系统 |
| 形变目标 | Morph Target | 顶点级别的形变数据（面部表情等）|

## N

| 术语 | 英文 | 说明 |
|------|------|------|
| 通知 | Notify | 见"动画通知" |
| 通知状态 | Notify State | 有 Begin/Tick/End 的持续段通知 |

## P

| 术语 | 英文 | 说明 |
|------|------|------|
| 物理动画 | Physical Animation | 动画与物理混合的次级运动系统 |
| 物理资产 | Physics Asset (PhAT) | 定义角色物理碰撞体和约束的资产 |
| 姿势资产 | Pose Asset | 存储命名静态姿势的资产（常用于面部）|
| 姿势搜索 | Pose Search | UE5 Motion Matching 的底层技术 |

## R

| 术语 | 英文 | 说明 |
|------|------|------|
| 参考姿势 | Reference Pose | 骨骼的静态基准姿势（T-Pose 或 A-Pose）|
| 动画重定向 | Retargeting | 将动画从一套骨骼应用到另一套骨骼 |
| 根运动 | Root Motion | 由根骨骼位移驱动角色实际移动的技术 |

## S

| 术语 | 英文 | 说明 |
|------|------|------|
| 次级运动 | Secondary Motion | 配件/布料在主骨骼运动时的物理跟随动画 |
| 骨骼 | Skeleton | 定义骨骼层级和命名的 UE 资产 |
| 骨骼控制 | Skeletal Control | AnimGraph 中直接修改骨骼变换的节点类 |
| 骨骼网格体 | Skeletal Mesh | 带骨骼绑定的 3D 网格体资产 |
| 蒙皮 | Skinning | 将骨骼变换应用到网格顶点的过程 |
| 蒙皮权重 | Skin Weights | 每个顶点受各骨骼影响的权重值 |
| 插槽 | Socket | 骨骼上的具名挂载锚点 |
| 状态机 | State Machine | 管理动画状态和转换的可视化逻辑系统 |
| 步幅扭曲 | Stride Warping | 动态调整角色步幅以匹配运动速度的技术 |
| 同步组 | Sync Group | 使多个动画步频保持同步的机制 |

## T

| 术语 | 英文 | 说明 |
|------|------|------|
| 变换 | Transform | 位置 + 旋转 + 缩放的组合（FTransform）|
| 过渡规则 | Transition Rule | 状态机中控制状态切换条件的逻辑 |

## W

| 术语 | 英文 | 说明 |
|------|------|------|
| 工作线程 | Worker Thread | UE5 中执行 AnimGraph 评估的后台线程 |
| 世界空间 | World Space | 相对于游戏世界原点的绝对坐标空间 |
