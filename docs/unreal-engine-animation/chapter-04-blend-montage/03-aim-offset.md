# 4.3 Aim Offset 深度解析

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 25 分钟

---

## 一、Aim Offset 是什么

Aim Offset 是特殊的 Blend Space，专门用于将上半身（头部、躯干、手臂）的**叠加（Additive）旋转**根据瞄准方向混合：

```
工作原理：

Base Pose（角色前向待机）
  + Aim_Up      （仰视 90°时的叠加旋转）× Pitch_Weight
  + Aim_Down    （俯视 -90°时的叠加旋转）× (-Pitch_Weight)
  + Aim_Left    （向左 -90°时的叠加旋转）× (-Yaw_Weight)
  + Aim_Right   （向右 +90°时的叠加旋转）× Yaw_Weight
  = 最终混合姿势

参数：
  X 轴：Yaw（偏航）  -90° ~ +90°（左 ~ 右）
  Y 轴：Pitch（俯仰）-90° ~ +90°（下 ~ 上）
```

---

## 二、Aim Offset 动画的制作要求

```
关键：Aim Offset 中的动画必须是 Additive 类型！

在 DCC 软件中制作：
  1. 以角色正前方平视为基准姿势（0,0）
  2. 制作 9 个方向的姿势：
     (-90,-90) 左下 | (0,-90) 正下 | (90,-90) 右下
     (-90,  0) 正左 | (0,  0) 正前 | (90,  0) 正右
     (-90, 90) 左上 | (0, 90) 正上 | (90, 90) 右上
  
  3. 每个方向姿势只修改上半身骨骼（脊椎、颈部、头部、手臂）
  4. 导入 UE 后，动画类型设为 Additive（相对于参考姿势）

在 UE 导入设置中：
  Additive Anim Type: Mesh Space
  Reference Pose: Selected animation frame
  Reference Frame: 0（基准帧，通常是正前平视帧）
```

---

## 三、创建 Aim Offset 资产

```
内容浏览器 → 右键 → Animation → Aim Offset
选择 Skeleton

配置轴：
  Horizontal Axis（X）：Yaw,  -90 ~ 90
  Vertical Axis（Y）：  Pitch, -90 ~ 90

放置采样点（最少 9 个）：
  (0, 0)      → AO_Center（中心，正前平视）
  (90, 0)     → AO_Right
  (-90, 0)    → AO_Left
  (0, 90)     → AO_Up
  (0, -90)    → AO_Down
  (90, 90)    → AO_RightUp
  (-90, 90)   → AO_LeftUp
  (90, -90)   → AO_RightDown
  (-90, -90)  → AO_LeftDown
```

---

## 四、在 AnimGraph 中配置 Aim Offset

```
节点配置：

[State Machine（下半身运动）]
        │
        ▼
[Layered Blend Per Bone]
  Base Pose: ← 上面的 State Machine 输出
  ─────────────────────────────────────────
  注意：Aim Offset 要用 Apply Mesh Space Additive 节点
        不是普通的 Apply Additive！
        
[State Machine 运动姿势]
        │
[Apply Mesh Space Additive]
  Base Pose: ← State Machine
  Additive:  ← [Aim Offset Player]
                  Yaw:   AnimInstance.AimYaw
                  Pitch: AnimInstance.AimPitch
  Alpha: 1.0
        │
        ▼
[Output Pose]
```

---

## 五、Aim Offset 的性能注意

```
性能对比：
  Apply Additive（骨骼空间）：  较快
  Apply Mesh Space Additive：   较慢（需要转换到 Mesh 空间）
  
为什么 Aim Offset 必须用 Mesh Space？
  普通叠加（Bone Space）：旋转相对于父骨骼，转弯时手枪方向会偏
  Mesh Space 叠加：旋转相对于 Mesh 组件，转弯时上半身方向保持世界空间正确

优化：
  如果角色不旋转（如俯视角固定方向角色），可以用普通 Apply Additive
  一般角色必须用 Mesh Space
```

---

## 六、AimOffset 平滑处理

```cpp
// 在 AnimInstance 中平滑 Aim 方向，防止抖动
void UMyAnimInstance::NativeThreadSafeUpdateAnimation(float Dt)
{
    // 对 AimPitch 做平滑（消除鼠标抖动）
    SmoothedAimPitch = FMath::FInterpTo(SmoothedAimPitch, AimPitch, Dt, 15.0f);
    SmoothedAimYaw   = FMath::FInterpTo(SmoothedAimYaw,   AimYaw,   Dt, 15.0f);
    // AnimGraph 中 Aim Offset 使用 SmoothedAimPitch/SmoothedAimYaw
}
```

---

## 七、延伸阅读

- 🔗 [Aim Offset 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/aim-offset-in-unreal-engine)
- 🔗 [Additive Animation 详解](https://dev.epicgames.com/documentation/en-us/unreal-engine/additive-animation-in-unreal-engine)
- 📄 [4.6 Additive 叠加动画](./06-additive-animation.md)
