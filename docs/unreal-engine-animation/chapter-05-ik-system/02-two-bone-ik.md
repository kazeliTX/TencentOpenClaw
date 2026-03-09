# 5.2 Two-Bone IK 完全指南

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、Two-Bone IK 原理

Two-Bone IK 是最常用的 IK 节点，解决"两段骨骼 + 一个末端"的经典问题：

```
骨骼链结构：
  Root Bone（根）→ Mid Bone（中间关节）→ End Bone（末端）
  
  例（腿部）：
    thigh_l（大腿）→ calf_l（小腿）→ foot_l（脚）
  
  例（手臂）：
    upperarm_r（上臂）→ lowerarm_r（前臂）→ hand_r（手）

求解目标：
  给定 Effector Target（效应器目标）= 希望末端到达的位置
  给定 Pole Target（极向目标）= 指定中间关节弯曲方向
  → 输出：每根骨骼的旋转量（使末端尽量靠近目标）
```

### 几何求解

```
已知：
  L1 = Root → Mid 骨骼长度（大腿长）
  L2 = Mid → End 骨骼长度（小腿长）
  D  = Root → Target 距离

求解余弦定理：
  cos(θ_mid) = (L1² + L2² - D²) / (2 × L1 × L2)
  cos(θ_root) = (L1² + D² - L2²) / (2 × L1 × D)
  
  θ_mid  = 中间关节弯曲角（膝盖/肘部）
  θ_root = 根骨骼旋转角（大腿/上臂）

Pole Target 决定弯曲平面：
  弯曲平面 = Root → Target 向量 与 Root → PoleTarget 向量 张成的平面
  中间关节在这个平面内朝 PoleTarget 方向弯曲
```

---

## 二、在 AnimGraph 中配置 Two-Bone IK

```
节点参数：

IK Bone:       foot_l         （末端骨骼，IK 要控制的目标）
Allow Stretching: false        （不允许骨骼拉伸，保持自然）
  Start Stretch Ratio: 1.0    （开始拉伸的距离比例）
  Max Stretch Scale:   1.2    （最大拉伸倍数）

Effector Transform:            （目标变换，Component Space）
  Translation: (X, Y, Z)      （目标位置，来自 AnimBP 变量）
  Rotation:    (Pitch, Yaw, Roll)（末端骨骼旋转，通常是地面法线）
  Scale:       (1,1,1)
  
  Location Target Space: World Space 或 Component Space（看具体需求）
  Rotation Target Space: Component Space

Joint Target:                  （Pole Target 极向目标）
  Translation: (X, Y, Z)      （膝盖/肘部朝向的参考点）
  Maintain JointOffset: ✅    （保持关节相对于目标的偏移）

Alpha: 1.0                    （IK 权重，1.0 = 完全 IK）

Take Rotation From Effector Target: ✅
  （末端骨骼旋转由效应器目标决定，否则保持 FK 旋转）
```

---

## 三、Pole Target 设置技巧

```
膝盖 Pole Target（腿部 IK）：
  位置：角色正前方，膝盖高度位置
  例：character.location + forward * 300 + up * knee_height
  
  公式：
    PolePos = (foot_world + pelvis_world) * 0.5     // 膝盖高度参考
    PolePos += character.GetActorForwardVector() * 150 // 向前偏移
  
肘部 Pole Target（手臂 IK）：
  持枪时：肘部偏后偏下（角色背后侧方）
  格斗时：肘部偏外（自然出拳方向）
  
  通常使用专门的"IK 骨骼"：
    ik_hand_gun 骨骼作为手部效应器
    hand_pole_l 骨骼作为左肘 Pole Target
    → 这些骨骼跟随角色但不影响蒙皮
```

---

## 四、C++ 中动态设置 IK 目标

```cpp
// 在 AnimInstance 中声明 IK 变量
UPROPERTY(BlueprintReadOnly, Category="IK")
FVector FootIK_LeftTarget  = FVector::ZeroVector;

UPROPERTY(BlueprintReadOnly, Category="IK")
FVector FootIK_RightTarget = FVector::ZeroVector;

UPROPERTY(BlueprintReadOnly, Category="IK")
FRotator FootIK_LeftRotation  = FRotator::ZeroRotator;

UPROPERTY(BlueprintReadOnly, Category="IK")
FRotator FootIK_RightRotation = FRotator::ZeroRotator;

UPROPERTY(BlueprintReadOnly, Category="IK")
float IKAlphaLeft  = 1.0f;
float IKAlphaRight = 1.0f;

// Two-Bone IK 节点直接读取上面的变量
// Effector → AnimInstance.FootIK_LeftTarget
// Alpha    → AnimInstance.IKAlphaLeft
```

---

## 五、常见问题排查

| 问题 | 原因 | 解决方案 |
|------|------|---------|
| 腿部穿地 | IK 目标位置太低 | 添加地面检测，目标位置 = 地面点 |
| 膝盖方向错误 | Pole Target 位置不对 | 检查 Pole Target 是否在预期方向 |
| 骨骼拉伸变形 | 目标超出骨骼长度 | 开启 Allow Stretching 或限制目标距离 |
| IK 抖动 | 目标位置每帧跳变 | 对目标位置做平滑插值 |
| 脚部旋转错误 | Take Rotation 配置有误 | 检查 Rotation Space 设置 |

---

## 六、延伸阅读

- 📄 [5.4 脚步 IK 完整实现](./04-foot-ik.md)
- 🔗 [Two Bone IK 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-two-bone-ik-in-unreal-engine)
