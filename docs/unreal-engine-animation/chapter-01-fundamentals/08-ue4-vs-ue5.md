# 1.8 UE4 vs UE5 动画系统差异对比

> **难度**：⭐⭐☆☆☆ | **阅读时间**：约 15 分钟
> 适合从 UE4 迁移过来的开发者，或需要理解 UE5 新特性的人

---

## 一、核心差异总览

| 维度 | UE4 | UE5 | 影响程度 |
|------|-----|-----|--------|
| **默认骨骼 Mannequin** | UE4 Mannequin（旧版）| Manny / Quinn（重新设计）| ⭐⭐⭐ |
| **IK 系统** | FABRIK（链式 IK）| Full Body IK（FBIK，多目标）| ⭐⭐⭐⭐ |
| **动画重定向** | 基于骨骼比例缩放 | IK Retargeter（精确，可视化）| ⭐⭐⭐⭐ |
| **Control Rig** | 实验性（不稳定）| 正式集成，功能完整 | ⭐⭐⭐⭐ |
| **物理引擎** | PhysX | Chaos Physics | ⭐⭐⭐ |
| **布料模拟** | Apex Cloth | Chaos Cloth | ⭐⭐⭐ |
| **动画压缩** | 多种算法手动选择 | ACL 为默认（压缩比更高）| ⭐⭐ |
| **Motion Warping** | 无 | 内置支持 | ⭐⭐⭐ |
| **Pose Search** | 无 | 实验性插件（5.3+）| ⭐⭐⭐⭐ |
| **ML Deformer** | 无 | 机器学习变形器 | ⭐⭐⭐ |
| **Linked Anim Layer** | 有限支持 | 完整支持，运行时可替换 | ⭐⭐⭐ |
| **多线程评估** | 可选 | 默认开启 | ⭐⭐ |
| **Thread Safe Update** | 不支持 | `NativeThreadSafeUpdateAnimation` | ⭐⭐⭐ |

---

## 二、Mannequin 迁移

### 2.1 骨骼结构变化

```
UE4 Mannequin 骨骼（部分）：
  root
  └── pelvis
      ├── spine_01 → spine_02 → spine_03
      │   ├── clavicle_l → upperarm_l → lowerarm_l → hand_l
      │   └── clavicle_r → ...
      ├── thigh_l → calf_l → foot_l → ball_l
      └── thigh_r → ...

UE5 Manny 骨骼（新增/改名）：
  • 新增了更多脊椎骨骼（spine_04, spine_05）
  • IK 骨骼命名调整（ik_foot_root, ik_foot_l/r, ik_hand_root 等）
  • 增加了虚拟骨骼（Virtual Bones）
  • 手指骨骼更完整（UE4 版手指较简化）
```

### 2.2 使用 IK Retargeter 从 UE4 迁移动画

```
迁移步骤：
1. 为 UE4 Mannequin 骨骼创建 IK Rig（定义骨骼链和效应器）
2. 为 UE5 Manny 骨骼创建 IK Rig
3. 创建 IK Retargeter：Source = UE4 IK Rig，Target = UE5 IK Rig
4. 在 Retargeter 中调整骨骼链对齐（T-Pose 对齐）
5. 使用 "Retarget Animation Assets" 批量转换所有动画
```

---

## 三、IK 系统对比

### 3.1 UE4：FABRIK

```
UE4 中的 FABRIK（Forward And Backward Reaching IK）：
  • 单链式 IK（一条骨骼链到一个目标）
  • 适合腿部/手臂简单 IK
  • 不支持多目标联动
  • 骨骼约束有限

在 AnimGraph 中使用 FABRIK 节点：
[Input Pose] → [FABRIK] → [Output Pose]
设置：
  End Effector（效应器位置）
  Root Bone（IK 链根骨骼）
  Tip Bone（IK 链末端骨骼）
  Precision（收敛精度）
  Max Iterations（最大迭代次数）
```

### 3.2 UE5：Full Body IK（FBIK）

```
UE5 Full Body IK：
  • 支持多个效应器同时求解
  • 全身骨骼协调运动（腿部 IK 自动带动腰部）
  • 基于 XPBD 求解器（物理精确）
  • 支持旋转约束、摆动约束
  • 支持骨骼刚度设置

FBIK 配置（IK Rig 编辑器）：
  Solvers → Add Solver → Full Body IK
  Effectors:
    • IK_Foot_L → 左脚目标
    • IK_Foot_R → 右脚目标
    • IK_Hand_L → 左手目标
    • IK_Hand_R → 右手目标
  
  Settings:
    Max Iterations: 20（越大越精确，越慢）
    Mass: 1.0（骨骼质量，影响求解偏好）
    Stretch: 0.0（允许骨骼拉伸的程度）
```

---

## 四、Control Rig 发展

### 4.1 UE4 的 Control Rig

UE4 的 Control Rig 处于实验性阶段，功能不完整，大多数团队仍在 DCC 软件（Maya）中完成绑定工作。

### 4.2 UE5 的 Control Rig

UE5 的 Control Rig 已经成为完整的**引擎内绑定系统**：

```
Control Rig 核心能力：
  ✅ 完整的视觉化绑定编辑器
  ✅ 支持 FK/IK 混合控制
  ✅ 可在 Sequencer 中直接制作动画
  ✅ 可在运行时作为 AnimGraph 节点使用
  ✅ 支持 Python 脚本批量操作
  ✅ 支持 Pose Library（姿势库）
  
工作流：
  传统：Maya 绑定 → FBX → UE 导入 → 动画师在 Maya 制作动画
  UE5：Control Rig 绑定（在引擎内）→ 动画师在 Sequencer 中直接制作
```

---

## 五、物理系统迁移

### 5.1 PhysX → Chaos

```
UE4（PhysX）:
  • 成熟稳定
  • 行业标准，与 Unity 相同
  • Cloth: Apex Cloth（功能有限）

UE5（Chaos）:
  • 更高精度的物理模拟
  • 原生 GPU 加速
  • Cloth: Chaos Cloth（支持更大规模布料）
  • Hair: Chaos Hair（发丝级物理）
  • Destruction: Chaos Destruction（实时破坏）

迁移注意：
  • 物理行为可能有轻微差异，需要重新调参
  • Physics Asset 的 Constraint 参数含义有变化
  • 布料模拟需要重新配置（Apex Cloth 资产不直接兼容）
```

---

## 六、代码层面的主要变化

```cpp
// UE5 新增：线程安全更新函数
// UE4 中没有这个函数，所有更新都在 NativeUpdateAnimation（游戏线程）
virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

// UE5 新增：Property Access 系统
// 用于在工作线程安全访问游戏线程的数据
UPROPERTY(BlueprintReadWrite, meta=(AnimPropertyAccess))
float Speed;  // 通过 PropertyAccess 系统确保线程安全

// UE5 变化：GetProxyOnGameThread 方式变化
// UE4:
FAnimInstanceProxy& Proxy = GetProxyOnGameThread<FAnimInstanceProxy>();
// UE5 同样支持，但推荐使用 PropertyAccess 替代手动代理访问

// UE5 新增：Linked Anim Layer 运行时切换
GetMesh()->LinkAnimClassLayers(NewLayerClass);
GetMesh()->UnlinkAnimClassLayers(OldLayerClass);
```

---

## 七、延伸阅读

- 🔗 [UE5 迁移官方指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-5-migration-guide)
- 🔗 [UE5 Full Body IK 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/fullbody-ik-in-unreal-engine)
- 🔗 [Control Rig 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/control-rig-in-unreal-engine)
- 🔗 [UE5 Unreal Animation Framework 解读（英文）](https://remremremre.github.io/posts/My-understanding-of-Unreal-Animation-Framework-in-5.6/)
