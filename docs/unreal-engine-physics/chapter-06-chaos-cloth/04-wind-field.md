# 6.4 风场配置（WindDirectionalSource）

> **难度**：⭐⭐⭐⭐☆

## 一、风场 Actor 类型

```
UE5 提供的风场 Actor：

1. WindDirectionalSource（定向风）
   全局方向性风（如季风、恒定海风）
   覆盖整个关卡
   
   参数：
     Speed：风速（cm/s），10~500 是合理范围
     Strength：风力强度（影响布料、粒子）
     Min Gust Amt：最小阵风系数（0~1）
     Max Gust Amt：最大阵风系数（0~1）
     → 阵风 = 在 Speed × Min ~ Speed × Max 之间随机波动

2. PointWindSource（点风源）
   从某个点向外辐射风（爆炸/魔法气旋）
   影响半径内的布料

3. 自定义风场（Niagara 风场，见第十章）
   更复杂的湍流/龙卷风效果
```

## 二、WindDirectionalSource 实战设置

```
自然风效果（公园场景）：
  Speed         = 80 cm/s
  Strength      = 0.3
  Min Gust Amt  = 0.2
  Max Gust Amt  = 0.8
  → 微风，布料轻微摆动，偶有阵风

强风效果（暴风雪）：
  Speed         = 400 cm/s
  Strength      = 1.0
  Min Gust Amt  = 0.5
  Max Gust Amt  = 1.5
  → 旗帜剧烈翻腾，裙摆大幅摆动

Chaos Cloth 接收风力条件：
  布料必须启用 "Use Wind" 选项
  Cloth Editor → Simulation → Wind:
    Air Drag Coefficient：空气阻力系数（越大越容易被风吹）
    Wind Method：
      Accurate（精确）：基于法线计算真实气动力
      Legacy（旧版）：简化版，性能更好
```

## 三、运行时动态风场

```cpp
// 运行时修改风速/方向
void SetWindStrength(AWindDirectionalSource* Wind, float NewStrength)
{
    if (!Wind) return;
    UWindDirectionalSourceComponent* WC =
        Wind->GetComponent();
    WC->Speed    = NewStrength * 200.f;
    WC->Strength = NewStrength;
}

// 随时间变化的风（动态效果）
void Tick(float Dt)
{
    if (!WindSource) return;
    WindPhase += Dt * 0.5f;  // 风的相位，每2秒一个周期
    float GustedSpeed = BaseWindSpeed *
        (0.5f + 0.5f * FMath::Sin(WindPhase)
              + 0.2f * FMath::Sin(WindPhase * 3.7f));  // 多频叠加=更自然
    WindSource->GetComponent()->Speed = GustedSpeed;
    
    // 随机偏转风向（±15度）
    float Yaw = BaseWindYaw +
        FMath::Sin(WindPhase * 1.3f) * 15.f;
    WindSource->SetActorRotation(FRotator(0, Yaw, 0));
}
```

## 四、延伸阅读

- 📄 [6.5 布料碰撞](./05-cloth-collision.md)
- 📄 [代码示例：动态风场系统](./code/02_wind_system.cpp)
