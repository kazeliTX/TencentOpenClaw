# 6.9 运行时布料控制

> **难度**：⭐⭐⭐⭐⭐

## 一、布料模拟开关

```cpp
USkeletalMeshComponent* Mesh = GetMesh();

// 暂停布料（不销毁状态，性能优化）
Mesh->SuspendClothingSimulation();

// 恢复布料
Mesh->ResumeClothingSimulation();

// 重置布料到绑定姿势（瞬移后调用，防止布料拉伸）
Mesh->ResetClothTeleportMode = EClothingTeleportMode::Reset;
// 或：
Mesh->ForceClothNextUpdateTeleportReset();

// 瞬移后布料渐进恢复（比直接 Reset 更柔和）
Mesh->ForceClothNextUpdateTeleport();
// 注意区别：
//   TeleportReset：布料直接回到绑定姿势（像重新初始化）
//   Teleport     ：布料保持当前姿势，只是更新位置（不重置速度）
```

## 二、运行时参数修改

```cpp
// 获取 Clothing Simulation（Chaos 版）
UClothingSimulationInteractor* Interactor =
    Mesh->GetClothingSimulationInteractor();

if (UClothingInteractor* CI = Interactor ?
    Interactor->GetClothingInteractor(0) : nullptr)
{
    // 修改重力缩放（例：进入水下布料变慢）
    CI->SetAnimDriveSpringStiffness(0.0f);  // 动画驱动强度
    
    // 注意：Chaos Cloth 的运行时参数修改 API 在 UE5.1-5.3 仍较有限
    // 大多数参数需要在编辑器预设好多个 ClothingAsset，运行时切换资产
}

// 切换到不同的布料资产（不同参数预设）
// 方案：预先创建 "Summer.cloth" 和 "Winter.cloth" 两套参数
// 运行时通过 SetClothingAsset 切换
```

## 三、传送/瞬移后的布料处理

```cpp
// 传送角色时需要重置布料，否则布料会从旧位置拉伸到新位置
void TeleportCharacter(FVector NewLocation)
{
    // 先重置布料
    GetMesh()->ForceClothNextUpdateTeleportReset();
    
    // 然后再移动位置
    SetActorLocation(NewLocation, false, nullptr,
        ETeleportType::TeleportPhysics);
}

// 快速移动时（如跑步中大转弯）防止布料拉伸
void Tick(float Dt)
{
    FVector DeltaLoc = GetActorLocation() - LastLocation;
    float Speed = DeltaLoc.Size() / Dt;
    
    if (Speed > 1000.f)  // 速度超过 1000 cm/s
    {
        GetMesh()->ForceClothNextUpdateTeleport();
        // Teleport（非Reset）：保持相对姿势，只修正大幅位移
    }
    LastLocation = GetActorLocation();
}
```

## 四、延伸阅读

- 📄 [6.10 常见问题与调试](./10-cloth-common-issues.md)
- 📄 [代码示例：运行时布料控制](./code/01_cloth_runtime_control.cpp)
