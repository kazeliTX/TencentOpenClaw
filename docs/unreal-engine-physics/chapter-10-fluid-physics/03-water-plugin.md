# 10.3 Water Plugin 水体系统

> **难度**：⭐⭐⭐⭐☆

## 一、Water Plugin 核心组件

```
启用：Edit → Plugins → Water ✓ + Water Extras ✓

核心 Actor：
  WaterBody_Ocean：无限海洋（基于 Gerstner Wave）
  WaterBody_Lake：静态湖泊
  WaterBody_River：流动河流（沿样条线）
  WaterBody_Custom：自定义形状水体

WaterBodyActor 关键参数：
  Water Material：水面材质（内置 M_Ocean/M_Lake）
  Wave Amplitude：波浪振幅（cm）
  Wave Frequency：波浪频率
  Water Depth：水深（影响颜色/折射）
  
  Gerstner Waves：
    多层 Gerstner 叠加 → 真实感海浪
    最多 32 层叠加（性能开销 vs 效果权衡）
    通常 4~8 层足够
```

## 二、获取水面高度（C++）

```cpp
// 在任意位置查询水面高度
#include "WaterBodyActor.h"
#include "WaterSubsystem.h"

float GetWaterHeight(UWorld* World, FVector Position)
{
    UWaterSubsystem* WaterSS =
        World->GetSubsystem<UWaterSubsystem>();
    if (!WaterSS) return 0.f;

    // 找到该位置最近的水体
    TArray<AWaterBody*> WaterBodies;
    WaterSS->GetAllWaterBodies(WaterBodies);
    
    for (AWaterBody* WB : WaterBodies)
    {
        EWaterBodyQueryFlags Flags =
            EWaterBodyQueryFlags::ComputeLocation |
            EWaterBodyQueryFlags::IncludeWaves;
        
        FWaterBodyQueryResult Result =
            WB->QueryWaterInfoClosestToWorldLocation(
                Position, Flags);
        
        if (Result.IsInWater())
            return Result.GetWaterSurfaceLocation().Z;
    }
    return -TNumericLimits<float>::Max();
}
```

## 三、延伸阅读

- 📄 [10.4 流体与角色交互](./04-fluid-interaction.md)
- 📄 [10.5 浮力系统](./05-buoyancy.md)
