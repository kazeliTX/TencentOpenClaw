# 11.3 悬挂系统

> **难度**：⭐⭐⭐⭐⭐

## 一、悬挂参数详解

```
每个车轮独立配置悬挂：

SuspensionMaxRaise（上行程，cm）：
  轮子可以向上收缩的最大距离
  越大 = 越能吸收地面凸起
  典型值：轿车 10~15cm，越野车 20~30cm

SuspensionMaxDrop（下行程，cm）：
  轮子可以向下伸展的最大距离（离地时）
  越大 = 轮子保持接地的能力越强
  典型值：轿车 15~20cm，越野车 25~40cm

SpringRate（弹簧刚度，N/cm）：
  越大 = 越硬（运动型），越小 = 越软（舒适型）
  赛车：高刚度（硬悬挂，降低重心转移）
  SUV：低刚度（软悬挂，越野舒适）
  
  公式估算：SpringRate ≈ 车重(kg) × g / (4 × MaxDrop_m)
  800kg 轿车，后行程 0.15m：SpringRate ≈ 800×9.8/0.6 ≈ 13000 N/m = 130 N/cm

SuspensionDampingRatio（阻尼比）：
  0 = 无阻尼（一直弹跳）
  0.5 = 临界阻尼（最快停止振荡）
  1.0 = 过阻尼（慢慢回位）
  推荐：0.4~0.6（轿车），0.3~0.5（越野）
  
SpringPreload（弹簧预压力，N）：
  静止时弹簧已经承受的力
  用于调整车辆静止时的离地高度
```

## 二、视觉悬挂（AnimBP）

```
Vehicle AnimBP 节点：
  Wheel Handler Node：
    自动根据 WheelSetup 更新每个轮骨骼的位置/旋转
    → 轮子随悬挂上下移动 + 随速度旋转 + 随转向角旋转
    
  Wheel Handler 连接到 Output Pose 即可
  无需手动设置轮骨骼变换
```
