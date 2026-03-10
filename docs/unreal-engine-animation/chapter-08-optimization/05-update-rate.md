# 8.5 动画更新频率控制（URO）

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、URO（Update Rate Optimization）

```
URO 是 UE 内置的动画降频系统：
  根据角色距离和屏幕占用率，自动降低动画更新频率
  
  例：角色在 30m 外 → 每 2 帧更新一次动画（等效 30fps@60fps）
      角色在 50m 外 → 每 4 帧更新一次（等效 15fps）

注意：URO 和 Animation Budget Allocator 功能重叠
  UE5 项目推荐使用 Budget Allocator（更智能）
  URO 是旧版方案，仍然有效
```

---

## 二、URO 配置

```
在 SkeletalMeshComponent 中配置：

EnableUpdateRateOptimizations: ✅（开启 URO）
DisplayDebugUpdateRateOptimizations: ✅（调试时开启，显示频率）

AnimUpdateRateTick（关键结构体，C++ 中配置）：
  MaxEvalRateForInterpolation: 4   ← 最大降频倍数（每 4 帧更新一次）
  BaseNonRenderedUpdateRate: 1.0   ← 非渲染状态每秒更新次数
  BaseVisibleDistanceFactorThresholds:
    [0.4, 8]   ← screen size 0.4 时每 1 帧更新（全速）
    [0.2, 4]   ← screen size 0.2 时每 2 帧更新
    [0.1, 2]   ← screen size 0.1 时每 4 帧更新
```

---

## 三、URO + 插值（保持流畅感）

```
降频后动画可能不流畅（每 2 帧才更新，中间帧没有骨骼更新）
UE 提供插值补偿：

InterpolateSkippedFrames: ✅
  开启后：即使动画每 2 帧才评估一次，
          每帧仍然对骨骼位置做线性插值
  效果：远处角色动画流畅感保留，但 CPU 开销降低

注意事项：
  插值对"突然停止"类动画效果差（如受击停帧）
  对循环运动（走路/跑步）效果很好
```

---

## 四、C++ 手动控制更新频率

```cpp
// 根据距离动态设置动画 Tick 间隔
void AMyNPC::UpdateAnimTickInterval(float PlayerDistance)
{
    USkeletalMeshComponent* Mesh = GetMesh();
    if (!Mesh) return;

    float TickInterval;
    if      (PlayerDistance < 10.f)  TickInterval = 0.f;    // 每帧
    else if (PlayerDistance < 20.f)  TickInterval = 1.f/30; // 30fps
    else if (PlayerDistance < 40.f)  TickInterval = 1.f/15; // 15fps
    else if (PlayerDistance < 80.f)  TickInterval = 1.f/10; // 10fps
    else                             TickInterval = 1.f/5;  // 5fps（极远）

    Mesh->SetComponentTickInterval(TickInterval);
    
    // 同时调整动画 Tick Group 优先级
    if (PlayerDistance > 40.f)
        Mesh->SetComponentTickEnabled(Mesh->GetWorld()->GetDeltaSeconds() > TickInterval);
}
```

---

## 五、延伸阅读

- 📄 [代码示例：更新频率控制器](./code/02_update_rate_controller.cpp)
- 🔗 [URO 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine#updaterateoptimiztions)
