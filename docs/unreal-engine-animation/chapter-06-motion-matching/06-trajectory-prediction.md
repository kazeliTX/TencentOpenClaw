# 6.6 轨迹预测与运动特征

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 30 分钟

---

## 一、轨迹预测的意义

```
Motion Matching 的核心在于匹配"未来轨迹"：

没有未来轨迹预测的 MM：
  只匹配当前姿势 → 类似于每帧选最近姿势
  响应输入会有滞后感（动画先找到合适帧再转向）
  
有未来轨迹预测的 MM：
  查询向量包含"我接下来想往哪走"
  数据库中寻找"动画接下来的轨迹和我预期最接近"的帧
  → 动画预先开始转向，响应自然
```

---

## 二、Motion Trajectory 插件

```
UE5 的 Motion Trajectory 插件提供轨迹预测组件：

组件：Trajectory Component（添加到角色）
  自动根据角色输入预测未来轨迹
  存储过去帧的历史轨迹

配置：
  History Count: 5~10（保存过去多少帧的轨迹）
  Prediction Count: 5~10（预测未来多少帧）
  Prediction Interval: 0.1s（采样间隔）
  Max Speed: 600.0（最大预测速度限制）
```

---

## 三、轨迹预测算法

```
UE 的默认轨迹预测策略（Constant Velocity）：

原理：
  当前速度向量 V
  未来 t 秒后的位置 = 当前位置 + V × t
  
  适合直线运动，不适合急转弯
  
改进策略（SpringDamper 弹簧阻尼）：
  目标速度 = 输入方向 × MaxSpeed
  当前速度 → 目标速度（弹簧阻尼过渡）
  预测各时间点的中间速度 → 积分得到轨迹
  
  优点：转向时轨迹是弧线，更符合实际运动
  UE 推荐使用 SpringDamper 轨迹预测

示意图：
  角色向前跑，玩家突然输入向右：
  
  Constant Velocity 预测（直线）：
    ↑↑↑  （继续向前预测）
    但实际会转向右
  
  SpringDamper 预测（弧线）：
    ↑↑→→  （预测逐渐转向右）
    与实际运动更接近
    → Motion Matching 会提前选到转向动画
```

---

## 四、在 AnimInstance 中使用轨迹

```cpp
// 获取角色轨迹（用于 Motion Matching 查询向量）
void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    
    if (!CachedChar) return;
    
    // 获取 Trajectory 组件
    UCharacterTrajectoryComponent* TrajComp =
        CachedChar->FindComponentByClass<UCharacterTrajectoryComponent>();
    if (!TrajComp) return;

    // 获取轨迹数据（供 Pose Search 使用）
    // Motion Matching 节点会自动读取 Trajectory 组件
    // 无需手动传递，只要组件存在即可

    // 也可以手动获取轨迹用于调试
    FPoseSearchQueryTrajectory Trajectory = TrajComp->GetTrajectory();
    // Trajectory.Samples 包含历史 + 未来轨迹点
    // 每个点：Position, Facing, Time
}
```

---

## 五、轨迹可视化调试

```
在编辑器 PIE 模式下调试轨迹：

1. 输入命令：a.TrajectoryDebugDraw 1
   → 在角色周围显示轨迹点（过去=红色，未来=绿色）

2. 查看预测是否符合预期：
   直线跑动：轨迹点应为直线
   转弯时：轨迹应为弧线（SpringDamper）

3. 调整 Trajectory 组件参数直到预测合理
```

---

## 六、延伸阅读

- 📄 [代码示例：轨迹生成器](./code/02_trajectory_generator.cpp)
- 🔗 [Motion Trajectory 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-trajectory-in-unreal-engine)
