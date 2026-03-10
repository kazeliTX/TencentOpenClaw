# 11.4 发动机与变速箱

> **难度**：⭐⭐⭐⭐⭐

## 一、发动机参数

```
FChaosVehicleEngineData：

  MaxRPM（最大转速）：
    普通轿车：6000~7000 RPM
    赛车：8000~12000 RPM
    
  MaxTorque（最大扭矩，N·m）：
    家用车：150~300 N·m
    肌肉车：500~700 N·m
    F1：~400 N·m（但极高转速补偿）
    
  TorqueCurve（扭矩曲线）：
    X = RPM（归一化 0~1）
    Y = 扭矩倍率（0~1）
    自定义曲线：
      低转速区（0~0.3）：0.5~0.7（低速扭矩）
      中转速区（0.4~0.7）：1.0（峰值扭矩）
      高转速区（0.8~1.0）：0.6~0.8（功率平台）
      
  EngineIdleRPM：怠速（800~1200 RPM）
  EngineRevDownRate：松油门后转速下降速率
  EngineRevUpRate：踩油门后转速上升速率
```

## 二、变速箱配置

```
FChaosVehicleTransmissionData：

  bUseAutomaticGears：true = 自动挡，false = 手动挡
  
  ForwardGearRatios：每个档位的齿轮比数组
    [2.8, 1.9, 1.3, 1.0, 0.8, 0.6]（6挡）
    一档比最大（低速大扭矩），高档比最小（高速省油）
    
  ReverseGearRatios：倒挡比 [2.5]
  
  FinalRatio：终传比（所有档位再乘一个倍率）
    增大 FinalRatio → 加速更快但极速降低
    降低 FinalRatio → 加速慢但极速提高
    
  GearChangeTime：换挡时间（秒，0.5~1.0 s）
  
  ChangeUpRPM：自动升挡的 RPM（通常 MaxRPM × 0.85）
  ChangeDownRPM：自动降挡的 RPM（通常 MaxRPM × 0.45）
  TransmissionEfficiency：传动效率（0.9~0.95）
```
