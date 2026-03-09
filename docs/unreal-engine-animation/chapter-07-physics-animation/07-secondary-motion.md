# 7.7 次级运动（Secondary Motion）

> **难度**：⭐⭐⭐☆☆ | **阅读时间**：约 20 分钟

---

## 一、次级运动概念

```
次级运动：跟随主体运动的辅助运动（有延迟和惯性）

例子：
  角色快跑时，背包带/配件随步伐弹跳
  快速转身时，长发/斗篷有惯性延迟
  马匹奔跑时，鬃毛和尾巴飘动

实现方式分级：
  1. 物理布料（最真实，开销最高）← 第五节已介绍
  2. Rigid Body 节点（物理刚体，中等）← 第八节
  3. Spring 骨骼（程序化弹簧，最快）← 本节
  4. 手K次级运动动画（最可控，最费时）
```

---

## 二、Spring Bone（弹簧骨骼）

```
AnimGraph 节点：Spring Arm（Bone Spring）

原理：
  指定一根"弹簧骨骼"（如 backpack_bone）
  该骨骼跟随父骨骼运动，但有弹簧延迟
  角色加速时：弹簧骨骼滞后（向后偏移）
  角色减速时：弹簧骨骼超前（向前偏移）

节点配置：
  Spring Bone:         backpack_bone
  Max Teleport Dist:   50.0   （超过此距离直接跳到目标，防止弹飞）
  Spring Stiffness:    50.0   （弹簧硬度，越高响应越快）
  Spring Damping:      4.0    （阻尼，越高振荡越小）
  
  Error Reset Threshold: 256.0 （超过此偏移强制重置）
  Limit Displacement:    true
  Max Displacement:      50.0  （最大偏移限制，单位 cm）
```

---

## 三、AnimDynamics（UE 程序化次级运动）

```
AnimDynamics 是更强大的程序化次级运动节点：
  支持多骨骼链（如脊椎、尾巴）
  支持线性/旋转限制
  支持重力影响
  
节点配置（背包示例）：
  Body Type:         Single Body（单骨骼）/ Chain（骨骼链）
  Bound Bone:        backpack_bone
  
  Linear Limits X:  (-5, 5)     （前后范围 cm）
  Linear Limits Y:  (-5, 5)     （左右范围 cm）
  Linear Limits Z:  (-10, 2)    （上下范围 cm，允许更多向下）
  
  Angular Limits:
    Swing1: (-20, 20)
    Swing2: (-20, 20)
    Twist:  (-10, 10)
  
  Linear Spring:     ✅
  Angular Spring:    ✅
  Spring Coefficient: 200.0
  Friction:           0.1
  Gravity Scale:      0.3    （背包受重力影响程度）
```

---

## 四、延伸阅读

- 🔗 [AnimDynamics 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animdynamics-in-unreal-engine)
- 🔗 [Spring Arm 节点文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-blueprint-bone-spring-in-unreal-engine)
