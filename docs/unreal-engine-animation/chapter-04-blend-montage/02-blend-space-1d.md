# 4.2 1D Blend Space 与简单混合

> **难度**：⭐⭐☆☆☆ | **阅读时间**：约 15 分钟

---

## 一、Blend Space 1D

1D Blend Space 只有一个参数轴，适合单维度混合：

```
典型应用：
  速度控制：Idle(0) → Walk(300) → Run(600) → Sprint(900)
  血量混合：Healthy(100%) → Injured(50%) → Critical(10%)
  疲劳混合：Fresh(0%) → Tired(50%) → Exhausted(100%)
  坡度混合：Flat(0°) → Uphill(45°) → Steep(90°)
```

### 创建 1D BlendSpace

```
内容浏览器 → 右键 → Animation → Blend Space 1D
  轴名称：Speed
  最小值：0.0
  最大值：600.0
  
采样点：
  0.0  → AS_Idle
  100.0 → AS_Walk_Start（可选，轻微过渡）
  300.0 → AS_Walk
  600.0 → AS_Run
```

---

## 二、BlendSpace 与 Blend Poses by Int 对比

```
什么时候用 BlendSpace，什么时候用 Blend Poses by Int？

BlendSpace（连续混合）：
  适合：参数连续变化（速度、方向、俯仰角）
  效果：丝滑过渡（自动插值）
  例：Speed 从 200 到 400，Walk 和 Run 同时播放并混合

Blend Poses by Int（离散选择）：
  适合：离散状态切换（武器类型、动作风格）
  效果：状态切换（带过渡时长）
  例：武器类型 0=空手、1=步枪、2=剑
  
如果参数是 bool/int，用 Blend Poses by Bool/Int
如果参数是连续 float，用 BlendSpace
```

---

## 三、C++ 运行时修改 BlendSpace 参数

```cpp
// 直接通过 AnimInstance 变量（推荐方式，AnimBP 节点读取）
void UMyAnimInstance::NativeUpdateAnimation(float Dt)
{
    GroundSpeed = CachedChar->GetVelocity().Size2D();
    // AnimBP 中的 BlendSpace Player 节点会自动读取 GroundSpeed
}

// 通过 FBlendSpaceNotifyTrigger（高级，特定通知）
// 一般不需要，直接用变量驱动即可
```

---

## 四、延伸阅读

- 📄 [4.3 Aim Offset 深度解析](./03-aim-offset.md)
- 🔗 [Blend Space 1D 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/blend-spaces-in-unreal-engine)
