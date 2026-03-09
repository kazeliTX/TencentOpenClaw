# 6.2 Root Motion 配置与工作流

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 30 分钟

---

## 一、动画资产配置

```
步骤一：在动画序列上启用 Root Motion

  打开 AnimSequence（如 A_Roll_Forward）
  Details 面板：
    Enable Root Motion: ✅
    Root Motion Root Lock: Ref Pose
    Force Root Lock: ☐（通常不勾，除非需要强制锁定）

步骤二：检查根骨骼是否有位移数据

  AnimSequence 编辑器 → 选择 root 骨骼
  时间轴上查看 root 的 Translation/Rotation 曲线
  如果 root 没有位移曲线 → 动画制作时没有烘焙 Root Motion
  需要在 DCC 工具（Maya/Blender）中重新导出
```

---

## 二、CharacterMovementComponent 配置

```cpp
// 在 ACharacter 构造函数中
AMyCharacter::AMyCharacter()
{
    // 启用 Root Motion 网络支持（重要！）
    GetCharacterMovement()->bAllowPhysicsRotationDuringAnimRootMotion = false;
    
    // 设置 Root Motion 来源（对应 AnimInstance 的 Root Motion Mode 设置）
    // 通常在 AnimBP 中配置，这里只是备注
}
```

### AnimInstance 配置

```
AnimBP 编辑器 → Class Defaults → Root Motion Mode：

选项：
  No Root Motion Extraction     → 无 Root Motion
  Ignore Root Motion            → 提取但不使用
  Root Motion from Everything   → 完整 Root Motion
  Root Motion from Montages Only → 推荐的混合方案（最常用）

推荐选择：Root Motion from Montages Only
  → 日常移动（Locomotion）：CharacterMovement 控制（BlendSpace）
  → 特殊动作（攻击/翻滚）：Montage Root Motion 控制
```

---

## 三、Root Motion Montage 实战配置

```
翻滚动作配置流程：

1. 创建 Montage：AM_Roll_Forward
   Source Animation: A_Roll_Forward（已启用 Root Motion）

2. Montage 设置：
   Blend In:  0.1s
   Blend Out: 0.15s

3. 在角色按下翻滚键时：
   PlayAnimMontage(AM_Roll_Forward)
   → Root Motion 自动驱动角色向前翻滚
   → 翻滚过程中角色仍然遵守碰撞
   → Montage 结束后恢复普通 Locomotion
```

---

## 四、Root Motion Scale（运行时缩放）

```cpp
// 运行时修改 Root Motion 的位移倍率
// 例：受伤状态翻滚距离缩短 30%
void AMyCharacter::PlayRollWithScale(float DistanceScale)
{
    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        // 临时修改 Root Motion 缩放（仅影响当前 Montage）
        Anim->SetRootMotionScale(DistanceScale);
        PlayAnimMontage(RollMontage);
    }
}

// 更精细的控制：通过 Montage 的 Root Motion Bias
// Montage Details → Root Motion Bias → Scale (X, Y, Z)
// X=1.5 表示前进方向拉长 1.5 倍
```

---

## 五、处理 Root Motion 与坡面/台阶

```
Root Motion 默认行为：
  CharacterMovement 应用 Root Motion 位移
  物理碰撞正常工作（不会穿墙/穿地）
  
  坡面处理：
    Root Motion 的水平位移 + CharacterMovement 的坡面跟随
    = 角色沿坡面滑动而不是穿地或飞空
  
  台阶处理：
    CharacterMovement.MaxStepHeight 控制可跨越台阶高度
    Root Motion 产生的向前位移遇台阶 → 自动爬升（≤ MaxStepHeight）

常见问题：Root Motion 动作在斜坡上位移量不对
  原因：Root Motion 是局部空间的，斜坡改变了运动方向
  解决：使用 Motion Warping 对 Root Motion 进行方向校正
```

---

## 六、延伸阅读

- 📄 [6.3 Root Motion 网络同步](./03-root-motion-network.md)
- 📄 [代码示例](./code/01_root_motion_controller.cpp)
- 🔗 [Root Motion 官方文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/root-motion-in-unreal-engine)
