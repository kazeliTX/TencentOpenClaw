# 9.7 SimulatedProxy 动画优化

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、SimulatedProxy 的特点

```
SimulatedProxy = 其他玩家角色在本地的表示

特点：
  位置数据来自网络（有延迟，有抖动）
  没有玩家直接控制（不需要输入响应）
  视觉平滑比状态精确更重要

优化方向：
  1. 位置插值（消除网络抖动）
  2. 动画降频（SimProxy 不需要 60fps 更新）
  3. 简化状态机（不需要完整的 AnimGraph）
```

---

## 二、SimProxy 位置平滑

```
问题：服务端每帧发送位置更新（实际上是 20~30 次/秒）
      客户端收到时：位置会跳跃（不流畅）

解决：UE 内置 SimProxy 平滑（CMC 自动处理）

配置：
  p.NetEnableMoveSmoothingParam 1   ← 必须开启
  p.SmoothNetUpdateRate 10          ← 插值速率（推荐 10~30）
                                        高 = 快速收敛但更抖
                                        低 = 平滑但延迟感更强

自定义平滑（C++ 中）：
  重写 SmoothClientPosition() 可实现更复杂的平滑逻辑
  如：使用弹簧阻尼插值代替线性插值
```

---

## 三、SimProxy 专用动画 LOD

```
SimProxy 可以比本地玩家用更低的 AnimGraph 复杂度：

// 在 AnimBP 中判断是否为 SimProxy
bool bIsSimProxy = OwnerCharacter && 
    OwnerCharacter->GetLocalRole() == ROLE_SimulatedProxy;

// 根据角色类型选择 AnimGraph 复杂度
if (bIsSimProxy)
{
    // SimProxy：简化路径
    // - 无 AimOffset（其他玩家的瞄准用 RemoteViewPitch 近似）
    // - 无 FBIK（脚步 IK 对远处角色意义不大）
    // - 降频：SetComponentTickInterval(1.f / 30.f)
}

// 在 AnimGraph 中：
// 使用 BlendBoolByBool 节点切换 SimProxy/Local 两套路径
```

---

## 四、SimProxy 网络更新频率

```
服务端发送 SimProxy 更新频率由 NetUpdateFrequency 决定：

// Character.h
NetUpdateFrequency = 60.f;       // 每秒最多发送 60 次更新（本地玩家）
MinNetUpdateFrequency = 10.f;    // 最少 10 次（距离远时降低）

// 距离远的 SimProxy：UE 自动降低更新频率
// → 30m 外的角色可能只有 10~20 次/秒更新
// → 与动画降频配合使用效果更好

推荐 NetUpdateFrequency 设置：
  竞技射击（精度优先）：NetUpdateFrequency = 100
  RPG/开放世界：NetUpdateFrequency = 20~30
  背景 NPC：NetUpdateFrequency = 5~10
```

---

## 五、延伸阅读

- 🔗 [Actor 网络更新文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/actor-replication-in-unreal-engine)
