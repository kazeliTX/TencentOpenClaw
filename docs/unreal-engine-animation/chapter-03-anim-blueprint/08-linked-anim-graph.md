# 3.8 Linked AnimGraph 模块化设计

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、为什么需要 Linked AnimGraph

```
单一巨型 AnimBP 的问题（超过 50 个状态时）：
  1. 编辑器卡顿：节点太多，打开/保存慢
  2. 合并冲突：多人同时编辑同一个 AnimBP 文件
  3. 复用困难：无法跨角色复用部分逻辑
  4. 职责混乱：移动、战斗、IK 逻辑全混在一起

Linked AnimGraph（链接动画图表）解决方案：
  主 AnimBP 作为"协调者"
  功能模块独立为子 AnimBP（Linked AnimBP）
  通过接口（Animation Layer Interface）解耦
```

---

## 二、架构设计

```
推荐的模块化架构：

ABP_Character_Main（主 AnimBP）
  │
  ├── 调用 → ABP_Locomotion（地面移动模块）
  │           ├── 走路/跑步 Blend Space
  │           └── 上坡/下坡调整
  │
  ├── 调用 → ABP_Combat（战斗模块）
  │           ├── 近战攻击 State Machine
  │           └── 射击 State Machine
  │
  ├── 调用 → ABP_IK（IK 模块）
  │           ├── 脚步 IK
  │           └── 手部 IK
  │
  └── 调用 → ABP_Weapon（武器特定模块）
              ├── ABP_Rifle（步枪专用动画）
              ├── ABP_Sword（剑专用动画）
              └── ABP_Bow（弓专用动画）    ← 运行时动态切换！
```

---

## 三、实现步骤

### 3.1 创建动画层接口

```
1. 内容浏览器 → 右键 → Animation → Animation Layer Interface
   命名：ILI_Locomotion（ILI_ 前缀 = Interface Linked）

2. 在接口中定义层函数（不含实现，只有签名）：
   + Add Layer:
     FullBody()         → FPoseLink（返回完整身体姿势）
     UpperBody()        → FPoseLink（仅上半身）
     AdditiveLayers()   → FPoseLink（叠加层）
```

### 3.2 在主 AnimBP 实现接口

```
1. 主 AnimBP → Class Settings → Implemented Interfaces → + ILI_Locomotion

2. AnimGraph 中：
   搜索 "FullBody" → 插入 Linked Anim Layer 节点
   节点会自动显示接口中定义的层名

3. AnimGraph 结构：
   [Layered Blend Per Bone]
     Base:    ← [FullBody() Layer]    （全身，由子 AnimBP 实现）
     Blend 0: ← [UpperBody() Layer]   （上半身覆盖）
     Additive: ← [AdditiveLayers()]   （叠加层）
   → Output Pose
```

### 3.3 创建子 AnimBP 实现接口

```
1. 创建新 AnimBP → Parent Class: UMyAnimInstance（共享 C++ 数据）
   命名：ABP_Locomotion

2. ABP_Locomotion → Class Settings → Implemented Interfaces → + ILI_Locomotion

3. 实现 FullBody() 函数（AnimGraph 中）：
   SM_Locomotion（状态机）→ Output Pose（FullBody）

4. 如果不需要实现 UpperBody()，让它 pass-through 即可：
   Input Pose → Output Pose（直接连接，无修改）
```

### 3.4 运行时切换子 AnimBP

```cpp
// 在角色代码中动态切换武器动画模块
void AMyCharacter::EquipWeapon(EWeaponType WeaponType)
{
    TSubclassOf<UAnimInstance> WeaponAnimClass;

    switch (WeaponType)
    {
    case EWeaponType::Rifle:
        WeaponAnimClass = ABP_Rifle::StaticClass();
        break;
    case EWeaponType::Sword:
        WeaponAnimClass = ABP_Sword::StaticClass();
        break;
    default:
        WeaponAnimClass = ABP_Unarmed::StaticClass();
    }

    // 链接武器动画层（立即生效，无需重新初始化整个 AnimBP）
    GetMesh()->LinkAnimClassLayers(WeaponAnimClass);
}

// 卸载武器：恢复默认层
void AMyCharacter::UnequipWeapon()
{
    // 传 nullptr 恢复到接口的默认实现（如果有的话）
    GetMesh()->UnlinkAnimClassLayers(ABP_Rifle::StaticClass());
}
```

---

## 四、数据共享：子 AnimBP 如何访问角色数据

```
子 AnimBP 需要访问主 AnimBP 的变量（如 Speed、bIsAiming）

方案 A：共享父类（最简单，推荐）
  主 AnimBP 父类：UMyAnimInstance
  子 AnimBP 父类：UMyAnimInstance（相同！）
  → 子 AnimBP 可以直接读取 UMyAnimInstance 的所有变量

方案 B：Property Access 绑定
  子 AnimBP → Property Access → 绑定到 "Outer Anim Instance"（主 AnimBP）
  → 通过主 AnimBP 的变量路径访问数据

方案 C：AnimBP 间接口传递
  主 AnimBP 通过 LayerParams 传递数据（高级，适合复杂项目）
```

---

## 五、常见模块划分建议

| 子 AnimBP 名 | 职责 | 状态机 |
|-------------|------|-------|
| ABP_Locomotion | 地面移动、蹲伏 | SM_Ground |
| ABP_Airborne | 跳跃、坠落、飞行 | SM_Air |
| ABP_Combat_Melee | 近战攻击连段 | SM_Melee |
| ABP_Combat_Ranged | 射击、换弹 | SM_Ranged |
| ABP_IK | 脚步/手部 IK | - |
| ABP_Additive | 受击叠加、呼吸 | - |
| ABP_Vehicle | 载具驾驶姿势 | SM_Vehicle |

---

## 六、延伸阅读

- 🔗 [Animation Blueprint Linking 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-linking-in-unreal-engine)
- 🔗 [Lyra 示例项目动画架构分析](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-in-lyra-sample-game-in-unreal-engine)
- 🔗 [UE5 动画系统新特性（知乎）](https://zhuanlan.zhihu.com/p/671487108)
