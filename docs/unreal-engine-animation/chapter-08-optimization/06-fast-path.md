# 8.6 Animation Fast Path 与线程优化

> **难度**：⭐⭐⭐⭐☆ | **阅读时间**：约 25 分钟

---

## 一、Animation Fast Path

```
UE 动画系统有两种求值路径：

慢速路径（Slow Path）：
  AnimGraph 节点通过蓝图反射访问 AnimInstance 成员变量
  每次访问都有反射 overhead（类型查找、安全检查）
  不能在 Worker Thread 运行

快速路径（Fast Path）：
  AnimGraph 直接访问 AnimInstance 成员（无反射）
  可以在 Worker Thread 运行（并行）
  比慢速路径快 2~5 倍

触发快速路径的条件：
  ✅ 直接访问 AnimInstance 的 UPROPERTY 成员变量
  ✅ 变量类型为：float, bool, int32, FVector, FRotator, FTransform
  ❌ 触发蓝图函数调用 → 慢速路径
  ❌ 访问嵌套结构体（UMyStruct.Value）→ 慢速路径
  ❌ 访问数组元素 → 慢速路径
```

---

## 二、Fast Path 诊断

```
编辑器中检查 Fast Path 状态：
  AnimBP 编辑器 → 编译后
  节点上出现闪电图标 ⚡ → Fast Path
  节点上出现警告图标 ⚠️ → 未使用 Fast Path，点击查看原因

常见警告及修复：

警告：Accessing member of struct
  原因：AnimBP 中访问 AnimInstance.MyStruct.Value（嵌套访问）
  修复：将 MyStruct.Value 展开为独立 UPROPERTY
    // 不好：
    UPROPERTY() FMyAnimData AnimData;  // 蓝图中 AnimData.Speed
    // 好：
    UPROPERTY() float Speed;           // 直接暴露 Speed

警告：Input pin connected to a function call
  原因：节点的输入通过蓝图函数计算
  修复：在 NativeUpdateAnimation 中预计算结果，存入 UPROPERTY

警告：Calling function in AnimGraph
  原因：直接在 AnimGraph 节点中调用蓝图函数
  修复：改为 NativeUpdateAnimation 预计算
```

---

## 三、Thread Safe Animation Updates

```
UE5 推荐将 AnimBP 更新逻辑迁移到 Worker Thread：

方法：
  AnimBP → 重写 BlueprintThreadSafeUpdateAnimation 事件
  （或 C++ 中重写 NativeThreadSafeUpdateAnimation）

限制（Worker Thread 中禁止的操作）：
  ❌ 访问其他 Actor 的组件
  ❌ 调用非线程安全的 UE 函数
  ❌ 修改物理状态
  ❌ Spawn 新 Actor

允许的操作：
  ✅ 数学计算（插值、向量运算）
  ✅ 读取已缓存到 AnimInstance 的数据
  ✅ 访问 AnimInstance 自身的变量
  ✅ 读取 UObject 的 const 数据

模式：
  Game Thread（NativeUpdateAnimation）：
    读取角色状态 → 缓存到 AnimInstance 变量
    
  Worker Thread（NativeThreadSafeUpdateAnimation）：
    对缓存数据做复杂计算（插值、状态判断）
    计算结果存入 AnimGraph 直接读取的 UPROPERTY
```

---

## 四、实践示例

```cpp
// 正确的线程分工示例
class UMyAnimInstance : public UAnimInstance
{
    // Game Thread 读取：
    UPROPERTY() float GroundSpeed;        // 从角色 Velocity 读取
    UPROPERTY() bool  bIsInAir;           // 从 CharacterMovement 读取
    UPROPERTY() AActor* LookAtTarget;     // 目标 Actor（Game Thread Only）
    
    // Worker Thread 计算结果（AnimGraph 直接读取，满足 Fast Path）：
    UPROPERTY() float LeanAngle;          // 倾斜角（计算密集，Worker Thread）
    UPROPERTY() FVector LookAtTargetCS;   // 注视目标（Component Space，Worker Thread）
    UPROPERTY() float BlendAlpha;         // 混合权重（Worker Thread）

    // Game Thread 更新（只读取 Actor 数据）
    virtual void NativeUpdateAnimation(float DeltaSeconds) override
    {
        ACharacter* C = Cast<ACharacter>(TryGetPawnOwner());
        if (!C) return;
        GroundSpeed = C->GetVelocity().Size2D();  // OK：Game Thread
        bIsInAir    = C->GetCharacterMovement()->IsFalling();
        LookAtTarget = C->GetCurrentTarget();     // 读取 Actor 引用
    }

    // Worker Thread 计算（只处理缓存数据）
    virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override
    {
        // 计算倾斜角（不访问其他 Actor，只用缓存的 GroundSpeed）
        float TargetLean = FMath::GetMappedRangeValueClamped(
            FVector2D(0, 600), FVector2D(0, 15), GroundSpeed);
        LeanAngle = FMath::FInterpTo(LeanAngle, TargetLean, DeltaSeconds, 5.f);
        
        // 注意：LookAtTarget 是 Actor 引用，Worker Thread 中不能安全访问
        // 应该在 Game Thread 中转换为 FVector 后再在这里使用
    }
};
```

---

## 五、延伸阅读

- 🔗 [Animation Fast Path 文档](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine#fastpath)
- 🔗 [Thread Safe Animation](https://dev.epicgames.com/documentation/en-us/unreal-engine/animation-optimization-in-unreal-engine#threadsafeanimationupdates)
