# 5.10 IK 系统性能优化

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 20 分钟

---

## 一、各类 IK 节点性能对比

```
（以 100 个角色 LOD0 为参考基准）

IK 类型                      单角色开销    100角色开销   优先级
────────────────────────────────────────────────────────────
Look At                       ~0.01ms       ~1ms          最低
Two-Bone IK（解析解）          ~0.02ms       ~2ms          很低
FABRIK（10骨骼，10迭代）        ~0.05ms       ~5ms          低
Spine IK (Control Rig)        ~0.08ms       ~8ms          中
Full Body IK（6效应器）        ~0.3ms        ~30ms         高
Control Rig（完整脚步+手部）   ~0.2ms        ~20ms         高
```

---

## 二、LOD 控制策略

```
不同 LOD 的 IK 配置建议：

LOD 0（近距离 < 10m）：
  ✅ 脚步 Two-Bone IK（双脚）
  ✅ 手部 Two-Bone IK（双手持枪）
  ✅ 头部 Look At
  ❌ Full Body IK（除非交互场景）

LOD 1（10~30m）：
  ✅ 脚步 IK（Alpha 降低到 0.5）
  ✅ 手部 IK（保持）
  ❌ 头部 Look At（禁用）

LOD 2（30~60m）：
  ❌ 所有 IK 禁用（LOD Threshold）
  使用纯 FK 动画

LOD 3（60m+）：
  极简动画或静止姿势
```

---

## 三、运动时减弱 IK

```cpp
// 高速运动时减弱脚步 IK（减少视觉突兀）
void UMyAnimInstance::NativeThreadSafeUpdateAnimation(float Dt)
{
    // 计算 IK Alpha
    float TargetAlpha;
    if (GroundSpeed < 10.f)
        TargetAlpha = 1.0f;          // 静止：完整 IK
    else if (GroundSpeed < 150.f)
        TargetAlpha = 0.8f;          // 慢走：大部分 IK
    else if (GroundSpeed < 400.f)
        TargetAlpha = 0.4f;          // 跑步：减弱 IK
    else
        TargetAlpha = 0.0f;          // 冲刺：禁用 IK

    FootIKAlpha = FMath::FInterpTo(FootIKAlpha, TargetAlpha, Dt, 5.f);
}
```

---

## 四、IK 地面检测优化

```
地面检测（Line Trace）是脚步 IK 最大的开销之一：

优化策略：
  1. 降低检测频率
     不需要每帧都检测！
     每 2~3 帧检测一次（对于慢速移动完全足够）
     用计数器控制：
     if (FrameCount++ % 3 == 0) { DoTrace(); }

  2. 检测通道优化
     使用专用 Trace Channel（FootIK）
     只对地形和静态物体响应
     排除动态物体（减少求解对象）

  3. 球形追踪 vs 线性追踪
     Line Trace：最快
     Sphere Trace：略慢但更准确（推荐半径 = 脚宽度的一半）

  4. 缓存结果
     如果角色没有移动（速度 < 1cm/s），重用上帧结果
```

---

## 五、优化清单

```
□ 所有 IK 节点设置 LOD Threshold
□ 高速运动时动态降低 FootIK Alpha
□ 地面检测频率控制（每 2~3 帧一次）
□ FBIK 只在交互状态激活（通过 Alpha 控制）
□ Control Rig 替代多个独立 IK 节点（单一求解更高效）
□ IK 目标全部做平滑插值（防止射线抖动）
□ OnlyTickPoseWhenRendered 确保屏幕外不更新
□ 角色不可见时 IK Alpha → 0
```

---

## 六、延伸阅读

- 🔗 [Foot Placement 性能指南](https://dev.epicgames.com/documentation/en-us/unreal-engine/foot-placement-in-unreal-engine)
- 🔗 [IK 优化实战（知乎）](https://zhuanlan.zhihu.com/p/1927668362969880273)
