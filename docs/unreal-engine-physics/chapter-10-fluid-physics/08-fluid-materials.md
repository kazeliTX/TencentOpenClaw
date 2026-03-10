# 10.8 流体材质技巧

> **难度**：⭐⭐⭐⭐☆

## 一、Water Plugin 内置材质参数

```
M_Ocean / M_Lake / M_River 参数（Material Instance）：

颜色相关：
  WaterColor_Near：近处水体颜色（浅蓝/绿）
  WaterColor_Far：远处水体颜色（深蓝）
  WaterScatterColor：散射颜色（水下光线颜色）
  
波纹相关：
  WaveAmplitude：波浪振幅（cm），影响顶点位移
  WaveSteepness：波浪陡峭度（0=正弦，1=尖锐波峰）
  NormalMap1/2：法线贴图（叠加两层 UV 速度不同）
  
视觉效果：
  Fresnel_Exponent：菲涅耳指数（越高水面倒影越强）
  Refraction：折射强度
  DepthFadeDistance：水深颜色渐变距离（cm）
  FoamTexture：泡沫贴图（浅水/岸边）
  FoamDepthThreshold：在水深 < 此值时显示泡沫（cm）
```

## 二、自定义水面材质要点

```
关键节点：
  WaterWaveFunction：
    输出水面世界坐标偏移（用于 World Position Offset）
    输出法线（用于 Normal 输入）
    
  SceneDepth：
    读取深度 Buffer → 计算焦散/水深效果
    (SceneDepth - PixelDepth) = 水深
    
  ScreenPosition：
    用于屏幕空间折射（读取场景颜色 + 偏移）
    
材质 Domain：Surface（非 Translucent，使用 DBuffer Decal 也可）
Blend Mode：Translucent（透明水面）
Shading Model：DefaultLit
```
